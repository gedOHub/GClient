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

int GClientLib::SCTPToServerSocket::Send(char* data, int lenght){
	int returnValue = 0;
	returnValue = sctp_send(this->Socket, data, lenght, NULL, 0);
	if (returnValue < 0){
		printf("%s", "Nepavyko isisusti duomenu");
	}
	return returnValue;
}

// Gaunam galimus adresu varaintus
void GClientLib::SCTPToServerSocket::GetAddressInfo(){
	// Laikinieji kintamieji
	struct addrinfo *result, hints;
	// Nustatom paieskos parametrus
	ZeroMemory(&hints, sizeof hints);
	// Pildoma pagal http://msdn.microsoft.com/en-us/library/windows/desktop/ms737530(v=vs.85).aspx
	// Ieskomp adreso tipas (IPv4, IPv6 ar kitas)
	hints.ai_family = AF_UNSPEC;
	// Sujungimo tipas
	hints.ai_socktype = SOCK_STREAM;
	// Transporto protokolas
	hints.ai_protocol = IPPROTO_SCTP;
	this->addrHints = &hints;
	// Gaunam adreso duomenis
	LPCSTR ip = this->IP->c_str();
	LPCSTR port = this->PORT->c_str();
	int rGetAddrInfo = getaddrinfo(ip, port, &hints, &result);
	if (rGetAddrInfo != 0){
		// Nepavyko gauti adreso duomenu
		printf("Nepavyko gauti adreso duomenu: %d\n", rGetAddrInfo);
		// Naikinam objekta
		freeaddrinfo(result);
	}
	this->addrResult = result;
}

int GClientLib::SCTPToServerSocket::Recive( int size ){
	sockaddr_in clientAddress;
	int AddrSize = sizeof(clientAddress);

	return recvfrom(this->Socket, &this->buffer[0], size, 0, (SOCKADDR *)& clientAddress, &AddrSize);
}