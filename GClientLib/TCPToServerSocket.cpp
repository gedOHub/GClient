#include "TCPToServerSocket.h"


GClientLib::TCPToServerSocket::TCPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket,
	fd_set* klaidingiSocket, SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) :
	GClientLib::ToServerSocket(ip, port, skaitomiSocket, rasomiSocket, klaidingiSocket, STOC, settings, tunnel){
	this->name = "TCPToServerSocket";
	this->maxPacketSize = TenMBofChar;
}

int GClientLib::TCPToServerSocket::Recive(int size){
	// Gaunu headeri
	int returnValue = recv(this->Socket, &this->buffer[0], sizeof(header), MSG_WAITALL);
	this->head = (struct header*)&this->buffer[0];
	printf("[%s] Laukiu: %d\n", this->name, ntohl(this->head->lenght));
	if (returnValue > 0) {
		// Gaunu tiek duomenu kiek nurodyta headeryje
		returnValue = returnValue + recv(this->Socket, &this->buffer[sizeof(header)], ntohl(this->head->lenght), MSG_WAITALL);
	}
	printf("[%s] Gauta is centrinio serverio: %d\n", this->name, returnValue);
	return returnValue;
}