#include "StdAfx.h"
#include "gNetSocket.h"

// Konstruktorius
gNetSocket::gNetSocket(string ip, string port, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket)
{
	// Tarnybines inforamcijos saugykla
	WSADATA wsaData;
	int wdaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (wdaResult != 0) {
		printf("WSAStartup failed: %d\n", wdaResult);
		// Nepavykus sunaikiname kintamaji
		return;
	}

	// Kintamuju priskirimas
	this->name = new char[20];
	this->name = "gNetSocket";
	this->read = false;
	this->write = false;
	this->IP = new string(ip);
	this-> PORT = new string(port);
	this->TAG = tag;
	this->Socket = INVALID_SOCKET;
	this->addrResult = nullptr;
	this->addrHints = nullptr;
	this->buffer = new char[TenMBofChar];
	// Socket sarasu kintamieji
	this->skaitomi = skaitomiSocket;
	this->rasomi = rasomiSocket;
	this->klaidingi = klaidingiSocket;
	// Kuriam SOCKET
	this->CreateSocket();
	// Pridedam socket prie sarasu
	FD_SET(this->Socket, skaitomiSocket);
	FD_SET(this->Socket, rasomiSocket);
	FD_SET(this->Socket, klaidingiSocket);
}

gNetSocket::gNetSocket(int socket, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket){
	// Tarnybines inforamcijos saugykla
	WSADATA wsaData;
	int wdaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (wdaResult != 0) {
		printf("WSAStartup failed: %d\n", wdaResult);
		// Nepavykus sunaikiname kintamaji
		return;
	}

	// Tikrinam ar kintamieji teisingi
	/*
	if(ip.empty() || tag < 0) {
		printf("Gauti netinkami duoemenys jungties kurimui");
		return;
	}
	*/

	// Kintamuju priskirimas
	this->Socket = INVALID_SOCKET;
	this->Socket = socket;
	this->TAG = tag;
	this->addrResult = nullptr;
	this->addrHints = nullptr;
	this->buffer = new char[FiveMBtoCHAR];
	// Socket sarasu kintamieji
	this->skaitomi = skaitomiSocket;
	this->rasomi = rasomiSocket;
	this->klaidingi = klaidingiSocket;;
	// Pridedam socket prie sarasu
	FD_SET(this->Socket, skaitomiSocket);
	FD_SET(this->Socket, rasomiSocket);
	FD_SET(this->Socket, klaidingiSocket);
}

// Destruktorius
gNetSocket::~gNetSocket(){
	this->CloseSocket();
}

// Grazina tag reiksme
int gNetSocket::GetTag(){
	return this->TAG;
}

int gNetSocket::GetPort(){
	sockaddr_in socketInfo;
	int ilgis = sizeof(socketInfo);
	if (getsockname(this->Socket, (struct sockaddr *)&socketInfo, &ilgis) != 0 && ilgis != sizeof(socketInfo)){
		return -1;
	}

	return ntohs(socketInfo.sin_port);
}

// Gaunam galimus adresu varaintus
void gNetSocket::GetAddressInfo(){
	// Laikinieji kintamieji
	struct addrinfo *result, hints;
	// Nustatom paieskos parametrus
	ZeroMemory( &hints, sizeof hints);
	// Pildoma pagal http://msdn.microsoft.com/en-us/library/windows/desktop/ms737530(v=vs.85).aspx
	// Ieskomp adreso tipas (IPv4, IPv6 ar kitas)
	hints.ai_family = AF_UNSPEC;
	// Sujungimo tipas
	hints.ai_socktype = SOCK_STREAM;
	// Transporto protokolas
	hints.ai_protocol = IPPROTO_TCP;
	this->addrHints = &hints;
	// Gaunam adreso duomenis
	LPCSTR ip = this->IP->c_str();
	LPCSTR port = this->PORT->c_str();
	int rGetAddrInfo = getaddrinfo(ip, port, &hints, &result);
	if( rGetAddrInfo != 0 ){
		// Nepavyko gauti adreso duomenu
		printf("Nepavyko gauti adreso duomenu: %d\n", rGetAddrInfo);
		// Naikinam objekta
		freeaddrinfo(result);
	}
	this->addrResult = result;
}
// Socket uzdarymas
// Pavykus usdaryti graziname TRUE, priesingai FALSE
bool gNetSocket::CloseSocket(){
	// Jungties uzdarimas
	int rClose = closesocket(this->Socket);
	if(rClose == SOCKET_ERROR){
		printf("Nepavyko inicijuoti socket uzdarymo: %d\n", WSAGetLastError());
		this->ShutdownSocket();
		WSACleanup();
		return false;
	}
	this->Socket = INVALID_SOCKET;
	// Salinu is nurodytu sarasu
	FD_CLR(this->Socket, skaitomi);
	FD_CLR(this->Socket, rasomi);
	FD_CLR(this->Socket, klaidingi);
	return true;
}

// Socket jungties naikinimas
void gNetSocket::ShutdownSocket(){
	// Jungties sunaikinimas
	int rShutDown = shutdown(this->Socket, SD_SEND);
	if(rShutDown == SOCKET_ERROR){
		this->Socket = INVALID_SOCKET;
		printf("Nepavyko inicijuoti socket uzdarymo: %d\n", WSAGetLastError());
		WSACleanup();
	}
	this->Socket = INVALID_SOCKET;
	//Salinu socketa is nurodytu sarasu
	FD_CLR(this->Socket, skaitomi);
	FD_CLR(this->Socket, rasomi);
	FD_CLR(this->Socket, klaidingi);
}

// Kuriam SOCKET
void gNetSocket::CreateSocket(){
	struct addrinfo *ptr;
	// Gaunam adreso duomenis
	this->GetAddressInfo();
	// Einam per visus gautus rezultatus
	for(ptr = this->addrResult; ptr != NULL; ptr=ptr->ai_next){
		// Bandom kurti socket
		this->Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		//printf("Socket dekrptorius: %d\n", this->Socket);
		// Sukurus socket iseinam is funkcijos
		if(this->Socket != INVALID_SOCKET) return;
		// Pranesam apie klaida kodel nepavyko sukurti socket
		printf("Klaida kuriant socket: %ld\n", WSAGetLastError());
	}
	// Jei nepavyko sukurti socketo
	printf("Klaida kuriant socket: %d\n", WSAGetLastError());
	freeaddrinfo(this->addrResult);
}

// Grazina SOCKET
SOCKET gNetSocket::GetSocket(){
	return this->Socket;
}

int gNetSocket::Send(char* data, int lenght){
	if(this->write){
		int rSend = 0;
		while(rSend != lenght){
			rSend =  rSend + send(this->Socket, &data[rSend], lenght - rSend, 0);
		}
		return rSend;
	}
	return 0;
}

void gNetSocket::SetRead(bool state){
	this->read = state;
}

char* gNetSocket::GetName(){
	return this->name;
}
