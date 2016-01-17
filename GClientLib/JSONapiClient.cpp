#include <winsock2.h>
#include <ws2tcpip.h>
#include "JSONapiClient.h"


using namespace GClientLib;

GClientLib::JSONapiClient::JSONapiClient(SOCKET socket, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket, JSONapi^ json) : gNetSocket(socket, tag, skaitomiSocket, rasomiSocket, klaidingiSocket){
	this->JSON = json;
	this->Socket = socket;
	this->name = "JSONapiClient";
	printf("[%s] SocketID: %d\n", this->name, this->Socket);
	this->write = true;
	this->read = true;
}

void GClientLib::JSONapiClient::Recive(SocketToObjectContainer^ container){
	using namespace std;

	if (this->read){
		// Gaunu duomenis
		const int rRecv = recv(this->Socket, this->buffer, FiveMBtoCHAR, 0);
		switch (rRecv){
		case 0:{
			printf("Klientas uzdare sujungima %d\n", this->Socket);
			container->DeleteBySocket(this->Socket);
			this->CloseSocket();
			break;
		}
		case SOCKET_ERROR:{
			printf("[%s]Klaida: %d sujungime %d \n", this->name, WSAGetLastError(), this->Socket);
			this->RemuveFromLists();
			break;
		}
		default:{
			// Super, gavau duomenis
			// Ieskau ka gavau

			// Pridedu, kad galeciau dirbti kaip su stringu.
			this->buffer[rRecv] = '\0';
			// Kuriu gautu duomenu stringa
			std::string RECIVE(this->buffer);

			// Isspausdinu ka gavau
			//cout << RECIVE << endl;

			// Ieskau uzklausos su options, del CROS
			size_t radauOptions = RECIVE.find("OPTIONS");
			// Ieskau index uzklausos, kad issiusciau nuoroda i WEB sasaja
			size_t radauGetIndex = RECIVE.find("GET / HTTP/1.1");

			if (radauOptions != string::npos){
				// Atejo OPTIONS uzklausa
				//cout << "OPTIONS" << endl;
				// http://www.w3.org/TR/cors/
				// Siunciu atgal atitnkamas antrastes
				char atsakasGet[] = "HTTP/1.1 200 OK\r\nServer: gNetClient\r\nAccess-Control-Allow-Origin: http://panel.jancys.net\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Max-Age: 1728000\r\nAccess-Control-Allow-Headers: x-requested-with\r\nVary: Accept-Encoding, Origin\r\nContent-Encoding: gzip\r\nContent-Length: 0\r\nKeep-Alive: timeout = 2, max = 100\r\nConnection: keep-alive\r\nContent-Type:text/plain\r\n\r\n";
				int rSend = send(this->Socket, atsakasGet, sizeof atsakasGet, 0);
				break;
			}

			// Radau GET uzklausa
			if (radauGetIndex != string::npos){
				// Peradresuoju i WebGUI tinklapi
				char atsakasGet[] = "HTTP/1.1 302 Found\r\nLocation: http://panel.jancys.net\r\n\r\n";
				int rSend = send(this->Socket, atsakasGet, sizeof atsakasGet, 0);
				break;
			}

			// Manau, kad atëjo JSON komanda
			// Formuoju uzklausa i serveri
			string response = this->JSON->readCommand(RECIVE, this->Socket);
			// Tikrinu ar reikia ka nors persiusti klientui
			if (!response.empty())
			{
				// Kazkas gauta, siusiu klientui
				int rSend = send(this->Socket, response.c_str(), response.size(), 0);
			}
			break;
		}
		}
	}
}

void GClientLib::JSONapiClient::ReciveJSONListAck(char* buffer, int dataSize, bool success)
{
	string response = this->JSON->FormatJSONListACK(buffer, dataSize, success);

	// Tikrinu ar reikia ka nors persiusti klientui
	if (!response.empty())
	{
		// Kazkas gauta, siusiu klientui
		int rSend = send(this->Socket, response.c_str(), response.size(), 0);
	}
}

