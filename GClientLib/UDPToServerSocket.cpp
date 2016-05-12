#include "UDPToServerSocket.h"


GClientLib::UDPToServerSocket::UDPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
	SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) :
	GClientLib::ToServerSocket(ip, port, skaitomiSocket, rasomiSocket, klaidingiSocket, STOC, settings, tunnel)
{
	this->name = "UDPToServerSocket";
	this->maxPacketSize = 65507;

	// Pradedu keep alive gyja
	//this->StartAckThread();
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

int GClientLib::UDPToServerSocket::Send(char* data, int lenght){
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(this->GetPort());
	serverAddress.sin_addr.s_addr = inet_addr(this->IP->c_str());
	int addrSize = sizeof(serverAddress);
	int returnValue = 0;

	printf("[%s] Reikia issiusti: %d\n", this->name, lenght);
	//this->logger->logDebug(this->className, "Reikia issiusti: " + std::to_string(size));
	/*
	while (returnValue != lenght) {
	while (!this->LockSending()){
	// Laukiu 10mili sekundziu
	Sleep(10);
	}
	// Priimu duomenis
	returnValue = sendto(this->Socket, &data[returnValue], lenght, 0, (SOCKADDR *)&serverAddress, addrSize);
	printf("[%s] Jau issiunciau: %d\n", this->name, returnValue);
	//this->logger->logDebug(this->className, "Jau issiunciau: " + std::to_string(returnValue));
	this->UnlockSending();
	}
	*/

	while (!this->LockSending()){
		// Laukiu 10mili sekundziu
		Sleep(10);
	}
	returnValue = sendto(this->Socket, &data[returnValue], lenght, 0, (SOCKADDR *)&serverAddress, addrSize);
	this->UnlockSending();
	printf("[%s] Issiusta i centrini serveri: %d\n", this->name, returnValue);
	if (sendto < 0){
		printf("[%s] Klaida gaunant duomenis. Klaidos kodas: %d\n", this->name, WSAGetLastError());
	}
	//this->logger->logDebug(this->className, "Issiusta: " + std::to_string(returnValue));
	return returnValue;
}

void GClientLib::UDPToServerSocket::SendKeepAlive(){
	// Pildau KEEP_ALIVE komanda
	char buf[sizeof(Command) + sizeof(header)];
	header* head = (struct header*)&buf[0];
	Command* alive = (struct Command*)&buf[sizeof(header)];
	alive->command = htons(UDP_ALIVE);
	head->tag = htons(0);
	head->lenght = htonl(sizeof(Command));

	DateTime now;

	while (this->live){
		this->Send(buf, sizeof(buf));
		// Laukiu 1s
		Thread::Sleep(1000);

		now = DateTime::Now;
		// Tikrinu ar ne senai gautas pranesimas is serverio
		/*
		if ((now - keepAliveLaikas).TotalSeconds > 3){
			// Gauta seniau :(
			System::Console::WriteLine("Prarastas rysis su serveriu\n");
			System::Console::WriteLine("Paskutini karta rysys su serveriu: \n");
			this->~UDPToServerSocket();
			this->StopAckThread();
		}
		*/

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

int GClientLib::UDPToServerSocket::Recive(int size){
	sockaddr_in serverAddress;
	int AddrSize = sizeof(serverAddress);

	int returnValue = 0;
	int MAXUDPPaketoDydis = 65507;
	// Paziuriu koks paketas atejo
	returnValue = recvfrom(this->Socket, &this->buffer[0], MAXUDPPaketoDydis, 0, (SOCKADDR *)& serverAddress, &AddrSize);
	printf("[%s] Gauta is centrinio serverio: %d\n", this->name, returnValue);
	if (returnValue < 0){
		printf("[%s] Klaida gaunant duomenis. Klaidos kodas: %d\n", this->name, WSAGetLastError());
	}
	// Nustatau headeri
	//header* head = (struct header*) & this->buffer[0];
	// Tikrinu ar gavau kazka
	//if (returnValue > 1) {
	// Atejo kazkas
	//paketoDydis = paketoDydis + ntohl(head->lenght);
	//printf("Reikia gauti: %d\n", paketoDydis);
	//this->logger->logDebug(this->className, "Reikai gauti: " + std::to_string(paketoDydis));
	// Gaunu headeri ir paketa
	//returnValue = recvfrom(this->Socket, &this->buffer[0], paketoDydis, MSG_WAITALL, (SOCKADDR *)& serverAddress, &AddrSize);
	//printf("Reikia gauti: %d\n", returnValue);
	//this->logger->logDebug(this->className, "Gauta: " + std::to_string(returnValue));
	//if (returnValue != paketoDydis) {
	//	printf("Gautas netinkamas dydis. Gauta: %d Laukta: %d\n", returnValue, paketoDydis);
	//this->logger->logError(this->className, "Gautas netinkamas dydis. "
	//	"Gauta: " + std::to_string(returnValue) + " Laukta: " +
	//	std::to_string(paketoDydis));
	//}
	//}
	// Grazinu ka gavau
	return returnValue;
}

void GClientLib::UDPToServerSocket::StartAckThread(){
	// Sukuriu thread aprasa
	this->ackThreadDelegate = gcnew ThreadStart(this, &UDPToServerSocket::SendKeepAlive);
	// Kuriu pacia gyja
	this->ackThread = gcnew Thread(ackThreadDelegate);

	// Pradedu gyjos darba
	this->ackThread->Start();
}

void GClientLib::UDPToServerSocket::StopAckThread(){
	this->live = false;
}

void GClientLib::UDPToServerSocket::Connect(){
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(INADDR_ANY);
	service.sin_port = htons(0);

	bind(this->Socket, (SOCKADDR *)&service, sizeof(service));
}