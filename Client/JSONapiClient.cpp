#include "stdafx.h"
#include "JSONapiClient.h"


JSONapiClient::JSONapiClient(SOCKET socket, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket) : gNetSocket(socket, tag, skaitomiSocket, rasomiSocket, klaidingiSocket){
	this->Socket = socket;
	this->name = "JSONapiClient";
	printf("[%s] SocketID: %d\n", this->name, this->Socket);
	this->write = true;  
	this->read = true;
}

void JSONapiClient::Recive(SocketToObjectContainer^ container){
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
				// Ieskau uzklausos su options, del CROS
				size_t radauOptions = RECIVE.find("OPTIONS");
				// Ieskau index uzklausos, kad issiusciau nuoroda i WEB sasaja
				size_t radauGetIndex = RECIVE.find("GET / HTTP/1.1");
				// Iesaku POST, kad gauciau ir persiusciau komandas i serveri
				size_t radauPost = RECIVE.find("POST");

				if (radauOptions != string::npos){
					char atsakasOptions[] = "HTTP/1.1 200 OK\r\nServer: gNet\r\nAccess-Control-Allow-Origin: http://gediminas.jancys.net\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: X-Requested-With, Content-Type\r\nAccess-Control-Max-Age: 1728000\r\nVary: Accept-Encoding, Origin\r\nContent-Encoding: gzip\r\nContent-Length: 0\r\nKeep-Alive: timeout=2, max=100\r\nConnection: Keep-Alive\r\nContent-Type: text/html\r\n\r\n";
					int rSend = send(this->Socket, atsakasOptions, sizeof atsakasOptions, 0);
					break;
				}

				// Radau GET uzklausa
				if (radauGetIndex != string::npos){
					// Peradresuoju i WebGUI tinklapi
					char atsakasGet[] = "HTTP/1.1 302 Found\r\nLocation: http://gediminas.jancys.net/panel/test1/\r\n\r\n";
					int rSend = send(this->Socket, atsakasGet, sizeof atsakasGet, 0);
					break;
				}

				// Radau POST uzklausa
				if (radauPost != string::npos){
					// Ieskau ar gavau duoemnis
					size_t radauContentLenght = RECIVE.find("Content-Length");
					if (radauContentLenght != string::npos){
						// Gavau kazkokius duomenis
						// Bandau issifruoti ka gavau
						// Ieskau duomenu kiekio sainio pabaigos
						size_t dataLineEnd = RECIVE.find('\r', radauContentLenght);
						// Gaunu duomenu kiekio sakini
						string ContentLength = RECIVE.substr((int)radauContentLenght, (int)dataLineEnd);
						// Gaunu tik duomenu kieki
						// Pagal http://www.cplusplus.com/forum/articles/9645/
						stringstream convert(ContentLength.substr((int)ContentLength.find(' ', 0) + 1, ContentLength.length()));
						// Duomenu ilgio kintamasis, int
						int contentLength;
						// Bandau konvertuoti
						if (!(convert >> contentLength)){
							// Nepavyko konvertuoti duomenu
							cerr << "Nepavyko konvertuoti duomenu ilgio reiksmes" << endl;
							break;
						}
						//Tesiu darba su konvertuotu kintamuoju
						// Paruosiu duomenis persiuntmui i serveri
						string dataToServer = RECIVE.substr(rRecv - contentLength, contentLength);
						// Uzdedu JSON headeri siuntimui i serveri
						int headerLength = PutJSONHeader(contentLength);
						// Paruosiu duomenis persiuntimui
						strcpy(&this->buffer[headerLength], dataToServer.c_str());
						int send = SendToGNetServer(container, contentLength + headerLength);

						cout << "Issiusta: " << send << endl;
					} else {
						// Negavau duomenu, nutraukiu darba
						break;
					}
				}
				break;
			} // END Default
		}
	}
}

void JSONapiClient::CreateSocket(){}

void JSONapiClient::SetRedirectUrl(String ^url){
	this->redirectUrl = url;
}

int JSONapiClient::PutJSONHeader(int dataLength){
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
int JSONapiClient::SendToGNetServer(SocketToObjectContainer^ container, int dataLenght){
	try{
		ToServerSocket^ server = (ToServerSocket^) container->FindByTag(0);
		cout << server->GetName() << endl;
		return server->Send(&this->buffer[0], dataLenght);
	} catch (exception e){
		return -1;
	}
}

