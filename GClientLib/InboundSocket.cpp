#include "InboundSocket.h"

using namespace GClientLib;

GClientLib::InboundSocket::InboundSocket(SOCKET socket, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket, int maxPacketSize) : gNetSocket(socket, tag, skaitomiSocket, rasomiSocket, klaidingiSocket){
	this->Socket = socket;
	this->name = "InboundSocket";
	//printf("[%s] SocketID: %d\n", this->name, this->Socket);
	this->write = true;
	this->maxPacketSize = maxPacketSize;
}

void GClientLib::InboundSocket::Recive(SocketToObjectContainer^ container){
	using namespace std;

	if(this->read){
		// Gaunu duomenis
		int rRecv = recv(this->Socket, &this->buffer[sizeof(header)], this->maxPacketSize - sizeof(header), 0);
		ToServerSocket^ toServer = (ToServerSocket^)container->FindByTag(Globals::CommandTag);
		switch(rRecv){
			case 0:{
				printf("[%s]Klientas uzdare sujungima %d\n", this->name, this->Socket);
				ToServerSocket^ toServer = (ToServerSocket^) container->FindByTag(Globals::CommandTag);
				toServer->CommandCloseTunnel(this->TAG);
				return;
			}
			case SOCKET_ERROR:{
				switch (WSAGetLastError()){
				case 10054:{
					printf("[%s] Socket %d uzdare sjungima. %d \n", this->name, this->Socket, WSAGetLastError());
					toServer->CommandCloseTunnel(this->TAG);
					return;
				}
				default:{
					printf("[%s] Klaida: %d sujungime %d \n", this->name, WSAGetLastError(), this->Socket);
					toServer->CommandCloseTunnel(this->TAG);
					return;
				}
				} // switch (WSAGetLastError()){
			}
			default:{
				head = (struct header *) &this->buffer[0];
				// Kuriu antraste
				head->tag = htons(this->TAG);
				head->lenght = htonl(rRecv);
				// Siunciam serveriui duomenis
				printf("[%s][Recive] Gavau duomenu: %d\n", this->name, rRecv);
				int rSend = toServer->Send(&this->buffer[0], (rRecv + sizeof header));
				printf("[%s][Recive] Issiunciau i serveri: %d\n", this->name, rSend);
			}
		}
	}
}

