#include "StdAfx.h"
#include "OutboundSocket.h"

using namespace GClientLib;

GClientLib::OutboundSocket::OutboundSocket(string ip, string port, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket) :gNetSocket(ip, port, tag, skaitomiSocket, rasomiSocket, klaidingiSocket){
	this->write = true;
	this->name = "OutboundSocket";
	this->Connect();
}

void GClientLib::OutboundSocket::Connect(){
	// Begam per visus gautus rezultatus ir bandom jungtis
	for(struct addrinfo * ptr = this->addrResult; ptr != NULL; ptr = ptr->ai_next){
		// Pasirienkam rezultata ir bandom jungtis
		int rConnect = connect(this->Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		// Tikrinam ar pavyko prijsungti
		if(rConnect == SOCKET_ERROR) {
			// Bandom tol, kol pavyks, delsiant viena minute
			printf("Klaida jungiatis i %s:%s kodas: %d\n", this->IP, this->PORT, WSAGetLastError());
			printf("Bandua jungtis prie %s:%s po minutes\n", this->IP, this->PORT);
			// Miegu 1 min
			Sleep(60000);
			// Jungiuosi
			this->Connect();
		} else {
			printf("Prisjungiau prie %s:%s\n", this->IP->c_str(), this->PORT->c_str());
			//cout << "Prisijungiau prie " << this->IP << ":" << this->PORT << endl;
			break;
		}
	}
}

void GClientLib::OutboundSocket::Recive(SocketToObjectContainer^ container){
	using namespace std;

	if(this->read){
		// Gaunu duomenis
		const int rRecv = recv(this->Socket, &this->buffer[sizeof(header)], FiveMBtoCHAR - sizeof(header), 0);
		switch(rRecv){
			case 0:{
				printf("Klientas uzdare sujungima %d\n", this->Socket);
				container->DeleteBySocket(this->Socket);
				this->CloseSocket();
				break;
			}
			case SOCKET_ERROR:{
				printf("Klaida: %d sujungime %d \n", WSAGetLastError(), this->Socket);
				break;
			}
			default:{
				head = (struct header *) &this->buffer[0];

				// Kuriu antraste
				head->tag = htons(this->TAG);
				head->lenght = htonl(rRecv);

				// Siunciam serveriui duomenis
				int rSend = container->FindByTag(0)->Send(&this->buffer[0], rRecv + sizeof(header));
				string status;
				if(rSend >  rRecv)
					status = "OK";
				else 
					status = "ERROR";
				//cout << "[" << this->name << "]" << status << " " << rRecv << " -> " << rSend <<  endl;
				break;
			}
		}
	}
}

