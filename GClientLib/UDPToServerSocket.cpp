#include "UDPToServerSocket.h"


GClientLib::UDPToServerSocket::UDPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
	SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) : 
	GClientLib::ToServerSocket(ip, port, skaitomiSocket, rasomiSocket, klaidingiSocket, STOC, settings, tunnel)
{
	this->name = "UDPToServerSocket";

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(INADDR_ANY);
	service.sin_port = htons(0);

	bind(this->Socket, (SOCKADDR *)&service, sizeof(service));

	// Pradedu keep alive gyja
	this->StartAckThread();
}

GClientLib::UDPToServerSocket::~UDPToServerSocket(){
	this->StopAckThread();
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
	serverAddress.sin_port = htons(this->GetPort());
	serverAddress.sin_addr.s_addr = inet_addr(this->IP->c_str());

	int rSend = 0;


	// Uzrakinu siuntimui objekta
	// Kol negaliu uzrakinti
	while (!this->LockSending()){
		// Laukiu 10mili sekundziu
		Sleep(10);
	}

	while (rSend != lenght && rSend != -1 ){
		rSend = sendto(this->Socket, &data[rSend], lenght, 0, (SOCKADDR *)&serverAddress, sizeof(serverAddress));
	}

	// Atrakinu siuntima
	this->UnlockSending();

	return rSend;
}

void GClientLib::UDPToServerSocket::SendKeepAlive(){
	// Kiek laiko nebuvo atsako, sekudnemis
	time_t now;

	while (this->live){
		// gaunu dabartini laika
		time(&now);
		// Tirkinu ar buvo gautas pranesiams pries maziau nei 10 s
		if (((clock() - keepAliveLaikas) / CLOCKS_PER_SEC) < 3){
			// Siunciu live paketa
			this->Send(this->buffer, 1);
			// Laukiu 1s
			Thread::Sleep(1000);
		}
		else {
			// Gauta seniau :(
			System::Console::WriteLine("Prarastas rysis su serveriu");
			this->~UDPToServerSocket();
			this->StopAckThread();
		}
		
	}
	//System::Console::WriteLine("SendKeepAlive baigia darba");
}

bool GClientLib::UDPToServerSocket::LockSending(){
	// Grazina uzrakto busena
	// Tirkinu ar uzrakinta
	if (this->locked == false)
	{
		// Uzrakinu
		this->locked = true;
	}
	return this->locked;
}

bool GClientLib::UDPToServerSocket::UnlockSending(){
	// Grazina uzrakto busena
	// Atrakinu siuntimo uzrakta
	this->locked = false;

	return this->locked;
}

int GClientLib::UDPToServerSocket::GetPort(){
	return std::atoi(this->PORT->c_str());
}

int GClientLib::UDPToServerSocket::Recive(){
	sockaddr_in clientAddress;
	int AddrSize = sizeof(clientAddress);

	return recvfrom(this->Socket, &this->buffer[0], TenMBofChar, 0, (SOCKADDR *)& clientAddress, &AddrSize);
}

void GClientLib::UDPToServerSocket::StartAckThread(){
	// Sukuriu thread aprasa
	this->ackThreadDelegate = gcnew ThreadStart(this, &UDPToServerSocket::SendKeepAlive);
	// Kuriu pacia gyja
	this->ackThread = gcnew Thread(ackThreadDelegate);
	
	//System::Console::WriteLine("Gyja pradeda darba");
	
	// Pradedu gyjos darba
	this->ackThread->Start();
}

void GClientLib::UDPToServerSocket::StopAckThread(){
	this->live = false;
}