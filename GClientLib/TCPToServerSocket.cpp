#include "TCPToServerSocket.h"


GClientLib::TCPToServerSocket::TCPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket,
	fd_set* klaidingiSocket, SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) :
	GClientLib::ToServerSocket(ip, port, skaitomiSocket, rasomiSocket, klaidingiSocket, STOC, settings, tunnel){
	this->name = "TCPToServerSocket";
}

int GClientLib::TCPToServerSocket::Recive(){
	int returnValue = -1;
	// Gaunu headeri
	returnValue = recv(this->Socket, &this->buffer[0], sizeof(header), MSG_WAITALL);
	// Nustatau headeri
	header* head = (struct header*) &this->buffer[0];
	// TIrkinu kiek reikai laukti duomenu
	int laukiama = ntohl(head->lenght);

	printf("Laukiama duomenu: %d\n", laukiama);

	if (laukiama > TenMBofChar){
		printf("Beda!! Buferis per mazas\n");
	}
	// Gauta daugiau nei headeis
	if (returnValue >= sizeof(header)) {
		// Nuskaitau likusius duomenis
		returnValue = recv(this->Socket, &this->buffer[sizeof(header)], ntohl(head->lenght), MSG_WAITALL);
		printf("Gauta duomenu: %d\n", returnValue);
	}
	// Tikrinu ar nera gauta per mazai
	if (laukiama != returnValue){
		// Gauta per mazai
		printf("Gautas netinkamas duomenu kiekis. Laukiama: %d gauta %d\n", laukiama, returnValue);
	}
	return returnValue;
	//return recv(this->Socket, &this->buffer[0], TenMBofChar, 0);
}