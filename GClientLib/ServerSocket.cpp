#include "StdAfx.h"
#include "ServerSocket.h"

using namespace GClientLib;

GClientLib::ServerSocket::ServerSocket(string ip, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket) :gNetSocket(ip, "0", tag, skaitomiSocket, rasomiSocket, klaidingiSocket){
	// Nustatau pradinius socke'o parametrus
	this->read = true;
	this->write = false;

	// Pradedu klausymasi
	this->Listen();
}

void GClientLib::ServerSocket::Listen(){
	// kviecaim bind funkcija
	this->Bind();
	// Bandom klausytis
	// SOMAXCONN - konstatna, kodel ji nezinau 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms739168(v=vs.85).aspx
	if(listen(this->Socket, SOMAXCONN) == SOCKET_ERROR){
		// Jei ivyko klaida
		printf( "Nepavyko klausytis %s socket: %ld\n", this->Socket, WSAGetLastError() );
		this->CloseSocket();
	}
}

void GClientLib::ServerSocket::Bind(){
	// Begam per visus gautus rezultatus ir bandom jungtis
	for(struct addrinfo * ptr = this->addrResult; ptr != NULL; ptr = ptr->ai_next){
		// Pasirienkam rezultata ir klausytis
		int rBind = bind(this->Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		// Tikrinam ar pavyko prijsungti
		if(rBind == SOCKET_ERROR) {
			// Bandom tol, kol pavyks, delsiant viena minute
			cout << "Klaida klausantis " << ptr->ai_addr << " Kodas: " <<  WSAGetLastError() << endl;
		} else break;
	}
}

int GClientLib::ServerSocket::Accept(SocketToObjectContainer^ container){
	// Sukuriam nauja SOCKET dekriptoriu
	struct sockaddr adresas;
	int adresasLen = sizeof(adresas);
	SOCKET newConnection = accept(this->Socket, &adresas, &adresasLen);
	//printf("[%s] Priemiau %d socketa\n", this->name, newConnection);
	if(newConnection == SOCKET_ERROR) {
		// Jei ivyko klaida kuriant dekriptoriu, pranesam
		printf("Klaida priimant sujungima: %d\n", WSAGetLastError());
		// Baigiam darba su deskriptorium, pereinam prie kito
		return newConnection;
	}
	// Nustatom maksimalu deskriptoriu
	if(Globals::maxD < (int)newConnection)
		Globals::maxD = newConnection;

	// Naujo sujungimo objektas
	InboundSocket^ guest = gcnew InboundSocket(newConnection, this->TAG, skaitomi, rasomi, klaidingi);
	
	// Salinu dabartini tag naudotoja
	// Besiklausanti socketa
	container->DeleteByTag(this->TAG);
	container->Add(guest);

	// --- Pridejimas prie sarasu ---
	// Pridedam prie besiklausanciu saraso
	FD_SET(newConnection, this->skaitomi);
	// Pridedam prie rasanciu saraso
	FD_SET(newConnection, this->rasomi);
	// Pridedam prie klaidu saraso
	FD_SET(newConnection, this->klaidingi);

	// Gaunam prisijungusiojo duomenis
	// Pagal https://support.sas.com/documentation/onlinedoc/sasc/doc750/html/lr2/zeername.htm
	struct sockaddr peer;
	int peer_len = sizeof(&peer);
	// Guanma kiento duomenis
	if(getpeername(newConnection, &peer, &peer_len) > 0){
		// Jei nepavyko gauti kliento duomenu
		printf("Nepavyko gauti kliento duomenu: %d\n", WSAGetLastError());
	} else {
		struct sockaddr_in *p = (struct sockaddr_in *) &peer;
		printf("Klientas %s prisijunge prie %d deskriptoriaus", inet_ntoa(p->sin_addr), (int) ntohs(p->sin_port));
	}

	// Siunciu i serveri CLIENT_CONNECT pranesima
	// Pildau CLIENT_CONNECT struktura
	clientConnectCommand* connect = (struct clientConnectCommand *) &this->buffer[sizeof header];
	connect->command  = htons(CLIENT_CONNECT);
	connect->tag = htons(guest->GetTag());
	// pildau paketo antraste
	this->head = (struct header*) &this->buffer[0];
	this->head->tag = htons(Globals::CommandTag);
	this->head->lenght = htonl(sizeof clientConnectCommand );

	// Issiunciu pranesima apir prisijungima
	container->FindByTag(Globals::CommandTag)->Send(&this->buffer[0], sizeof header + sizeof clientConnectCommand);

	return newConnection;
}

void GClientLib::ServerSocket::Recive(SocketToObjectContainer^ container){
	if(this->read){
		// Atejo nuajas sujungimas i bseiklausanti socketa
		if( this->Accept(container) == SOCKET_ERROR ) {
			printf("Nepavyko priimti jungties\n");
		}
	}
}
