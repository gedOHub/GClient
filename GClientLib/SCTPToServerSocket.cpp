#include "SCTPToServerSocket.h"
#include <ws2sctp.h>


GClientLib::SCTPToServerSocket::SCTPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
	SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) : ToServerSocket(ip, port, skaitomiSocket, rasomiSocket,
	klaidingiSocket, STOC, settings, tunnel)
{
	// Nustatau pavadinima
	this->name = "SCTPToServerSocket";
}


GClientLib::SCTPToServerSocket::~SCTPToServerSocket()
{
	printf("Obejktas %s baigia darba\n", this->name);
}

void GClientLib::SCTPToServerSocket::CreateSocket(){
	// Nustatau rezultato protokola
	this->addrResult->ai_protocol = IPPROTO_SCTP;
	// Kuriu socketa
	GClientLib::gNetSocket::CreateSocket();
	// Isvalom adreso informacija
	freeaddrinfo(this->addrResult);
}

int GClientLib::SCTPToServerSocket::Send(char* data, int lenght){
	int returnValue = 0;
	returnValue = sctp_send(this->Socket, data, lenght, NULL, 0);
	if (returnValue < 0){
		printf("%s", "Nepavyko isisusti duomenu");
	}
	return returnValue;
}


