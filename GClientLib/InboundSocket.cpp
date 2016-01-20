#include "InboundSocket.h"

using namespace GClientLib;

GClientLib::InboundSocket::InboundSocket(SOCKET socket, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket) : gNetSocket(socket, tag, skaitomiSocket, rasomiSocket, klaidingiSocket){
	this->Socket = socket;
	this->name = "InboundSocket";
	printf("[%s] SocketID: %d\n", this->name, this->Socket);
	this->write = true;
}

void GClientLib::InboundSocket::Recive(SocketToObjectContainer^ container){
	using namespace std;

	if(this->read){
		// Gaunu duomenis
		const int rRecv = recv(this->Socket, &this->buffer[sizeof(header)], FiveMBtoCHAR - sizeof(header), 0);
		switch(rRecv){
			case 0:{
				printf("[%s]Klientas uzdare sujungima %d\n", this->name, this->Socket);
				ToServerSocket^ toServer = (ToServerSocket^) container->FindByTag(Globals::CommandTag);
				toServer->CommandCloseTunnel(this->TAG);
				return;
				break;
			}
			case SOCKET_ERROR:{
				printf("[%s]Klaida: %d sujungime %d \n", this->name, WSAGetLastError(), this->Socket);
				ToServerSocket^ toServer = (ToServerSocket^)container->FindByTag(Globals::CommandTag);
				toServer->CommandCloseTunnel(this->TAG);
				return;
				break;
			}
			default:{
				head = (struct header *) &this->buffer[0];

				// Kuriu antraste
				head->tag = htons(this->TAG);
				head->lenght = htonl(rRecv);

				// Siunciam serveriui duomenis
				int rSend = container->FindByTag(Globals::CommandTag)->Send(&this->buffer[0], (rRecv + sizeof header));
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

