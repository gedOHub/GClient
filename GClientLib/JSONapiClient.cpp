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

			std::ostringstream responseStream;
			string data = "{ success: true, itemCount : 4, items : [{ id: 1, domain : 'GMC.LOCAL', pcname : 'GMC-GEDO', username : 'gedas' }, { id: 2, domain : 'GMC.LOCAL', pcname : 'GMC-TADO', username : 'tadas' }, { id: 3, domain : 'GMC.LOCAL', pcname : 'GMC-RAMO', username : 'ramas' }, { id: 4, domain : 'GMC.LOCAL', pcname : 'GMC-ROLANDO', username : 'asdasd' }] }";
			responseStream << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://panel.jancys.net\r\nAccept-Ranges:bytes\r\nServer: gNetClient\r\nContent-Length: ";
			responseStream << data.size() << "\r\nKeep-Alive: timeout=5, max=100\r\nConnection: keep-alive\r\nContent - type : application / json\r\n\r\n";
			responseStream << data;

			string rString = responseStream.str();

			int rSend = send(this->Socket, rString.c_str(), rString.size(), 0);
			cout << "Issiusta " << rSend << " is " << rString.size() << endl;
			break;
		}
		}
	}
}

int GClientLib::JSONapiClient::PutJSONHeader(int dataLength){
	// Kuriu antraste
	APIHeader* jsonHeader = (struct APIHeader *) &this->buffer[0];
	// Nustatau zyme
	jsonHeader->tag = htons(1);
	// Nustatau kiek daug bus siunciama duomenu
	jsonHeader->lenght = htonl(dataLength);
	// Uzdedama zyme
	jsonHeader->apiNumber = htons(1);
	// TAG'as per kuriuo reikai grazinti duomenis
	jsonHeader->returnTag = htons(this->TAG);
	return sizeof APIHeader;
}


