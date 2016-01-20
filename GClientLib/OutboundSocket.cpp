
#include "OutboundSocket.h"

using namespace GClientLib;

GClientLib::OutboundSocket::OutboundSocket(string ip, string port, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket, ToServerSocket^ server) :gNetSocket(ip, port, tag, skaitomiSocket, rasomiSocket, klaidingiSocket){
	this->server = server;

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
			printf("Klaida jungiantis i %s:%s kodas: %d\n", this->IP, this->PORT, WSAGetLastError());
			this->server->CommandCloseTunnel(this->TAG);
			return;
		} else {
			printf("Prisjungiau prie %s:%s\n", this->IP->c_str(), this->PORT->c_str());
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
				this->server->CommandCloseTunnel(this->TAG);
				return;
				break;
			}
			case SOCKET_ERROR:{
				printf("Klaida: %d sujungime %d \n", WSAGetLastError(), this->Socket);
				this->server->CommandCloseTunnel(this->TAG);
				return;
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
				if(rSend > rRecv)
				status = "OK";
				else
				status = "ERROR";
				//cout << "[" << this->name << "]" << status << " " << rRecv << " -> " << rSend <<  endl;
				break;
			}
		}
	}
}


