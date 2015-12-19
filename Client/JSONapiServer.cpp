#include "stdafx.h"
#include "JSONapiServer.h"


JSONapiServer::JSONapiServer(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket) :gNetSocket(ip, port, 0, skaitomiSocket, rasomiSocket, klaidingiSocket)
{
	this->Listen();
	this->read = true;
	this->write = true;
}
void JSONapiServer::Listen(){
	// kviecaim bind funkcija
	this->Bind();
	// Bandom klausytis
	// SOMAXCONN - konstatna, kodel ji nezinau 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms739168(v=vs.85).aspx
	if (listen(this->Socket, SOMAXCONN) == SOCKET_ERROR){
		// Jei ivyko klaida
		printf("Nepavyko klausytis %s socket: %ld\n", this->PORT, WSAGetLastError());
		this->CloseSocket();
	}
}
void JSONapiServer::Bind(){
	// Begam per visus gautus rezultatus ir bandom jungtis
	for (struct addrinfo * ptr = this->addrResult; ptr != NULL; ptr = ptr->ai_next){
		// Pasirienkam rezultata ir klausytis
		int rBind = bind(this->Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		// Tikrinam ar pavyko prijsungti
		if (rBind == SOCKET_ERROR) {
			// Bandom tol, kol pavyks, delsiant viena minute
			cout << "Klaida klausantis " << this->IP << ":" << this->PORT << " Kodas: " << WSAGetLastError() << endl;
		}
		else break;
	}
}
int JSONapiServer::Accept(SocketToObjectContainer^ container){
	// Sukuriam nauja SOCKET dekriptoriu
	struct sockaddr adresas;

	int adresasLen = sizeof(adresas);
	SOCKET newConnection = accept(this->Socket, &adresas, &adresasLen);
	//printf("[%s] Priemiau %d socketa\n", this->name, newConnection);
	if (newConnection == SOCKET_ERROR) {
		// Jei ivyko klaida kuriant dekriptoriu, pranesam
		printf("Klaida priimant sujungima: %d\n", WSAGetLastError());
		// Baigiam darba su deskriptorium, pereinam prie kito
		return newConnection;
	}
	// Nustatom maksimalu deskriptoriu
	if (Globals::maxD < (int)newConnection)
		Globals::maxD = newConnection;
	// Ieskau ToServer jungtis
	ToServerSocket^ server = (ToServerSocket^ )(container->FindByTag(0));

	// Naujo sujungimo objektas
	JSONapiClient^ guest = gcnew JSONapiClient(newConnection, server->GenerateTag(), skaitomi, rasomi, klaidingi);

	// Besiklausanti socketa
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
	if (getpeername(newConnection, &peer, &peer_len) > 0){
		// Jei nepavyko gauti kliento duomenu
		printf("Nepavyko gauti kliento duomenu: %d\n", WSAGetLastError());
	}
	else {
		struct sockaddr_in *p = (struct sockaddr_in *) &peer;
		printf("Klientas %s prisijunge prie %d deskriptoriaus\n", inet_ntoa(p->sin_addr), (int)ntohs(p->sin_port));
	}

	return newConnection;
}
void JSONapiServer::Recive(SocketToObjectContainer^ container){
	if (this->read){
		// Atejo nuajas sujungimas i bseiklausanti socketa
		if (this->Accept(container) == SOCKET_ERROR) {
			printf("Nepavyko priimti jungties\n");
		}
	}
}

void JSONapiServer::SetRedirectUrl(String ^url){
	this->redirectUrl = url;
}