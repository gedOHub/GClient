#include "UDPToServerSocket.h"


GClientLib::UDPToServerSocket::UDPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
	SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) : 
	GClientLib::ToServerSocket(ip, port, skaitomiSocket, rasomiSocket, klaidingiSocket, STOC, settings, tunnel)
{
	this->name = "UDPToServerSocket";
}

void GClientLib::UDPToServerSocket::CreateSocket(){
	// Kuriu socketa
	this->Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	// Struktura sauganti duomneis kaip siust duomenis i serveri
	
	if (this->Socket == INVALID_SOCKET) {
		// Pranesam apie klaida kodel nepavyko sukurti socket
		printf("Klaida kuriant socket: %ld\n", WSAGetLastError());
		return;
	}
}

void GClientLib::UDPToServerSocket::Connect(){
}

int GClientLib::UDPToServerSocket::Send( char* data, int lenght ){
	// Pildau serverio adreso struktura
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	//int p = System::Convert::ToInt32(this->PORT->c_str());
	serverAddress.sin_port = htons(1300);
	serverAddress.sin_addr.s_addr = inet_addr("79.98.30.72");

	int rSend = 0;
	while (rSend != lenght && rSend != -1 ){
		rSend = rSend + sendto(this->Socket, &data[rSend], lenght, 0, (SOCKADDR *)&serverAddress, sizeof(serverAddress));
	}
	return rSend;
}
