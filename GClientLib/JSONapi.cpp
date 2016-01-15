
#include "JSONapi.h"
#include <iostream>
#include <sstream>

using namespace std;

GClientLib::JSONapi::JSONapi(SettingsReader^ settings, ToServerSocket^ toServer){
	this->settings = settings;
	this->toServer = toServer;
}

std::string GClientLib::JSONapi::putHTTPheaders(string data)
{
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 200 OK\r\n";
	responseStream << "Access-Control-Allow-Origin: ";
	responseStream << this->settings->getSetting("webGUIAddress");
	responseStream << "\r\n";
	responseStream << "Accept-Ranges:bytes\r\n";
	responseStream << "Server: gNetClient\r\n";
	responseStream << "Content-Length: ";
	responseStream << data.size();
	responseStream << "\r\n";
	responseStream << "Keep-Alive: timeout = 5, max = 100\r\n";
	responseStream << "Connection: keep-alive\r\n";
	responseStream << "Content-type : application/json\r\n\r\n";
	responseStream << data;
	return responseStream.str();
}

// Metodas skirtas nauskaityti gauta komanda ir ja ivygdyti
std::string GClientLib::JSONapi::readCommand(string commandData, SOCKET clientSocket)
{
	// Kintamasis skirtas nustatyti reikalinga pozicija
	size_t pos;

	// Ieskau kokia komanda atejo
	// Tikrinu ar neatejo klientu sàraðo komanda
	if (commandData.find("clientList") != string::npos)
	{
		//Uzklausos pavizdys:
		//GET /?_dc=1452802220979&clientList=1&start=0&limit=20 HTTP/1.1
		//Host: 127.0.0.1:3000
		//Connection: keep-alive
		//Origin: http://panel.jancys.net
		//X-Requested-With: XMLHttpRequest
		//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36
		//Accept: */*
		//DNT: 1
		//Referer : http ://panel.jancys.net/
		//Accept - Encoding : gzip, deflate, sdch
		//Accept - Language : lt, en - US; q = 0.8, en; q = 0.6

		// Ieskau kurio puslapio nori gauti duomenis
		// Kintamasis skirtas nurodyti kokios komandos argumento ieskome
		string commandArgument = "clientList=";
		// Surandu kur slepiasi clientList
		pos = commandData.find(commandArgument);
		// Salinu uzklausos pradzia iki clientList kintamojo
		commandData.erase(0, pos + commandArgument.length());
		// Likusi uzklausa: 1&start=0&limit=20 HTTP/1.1....

		// Siusiu JSON list komanda i serveri
		// Isgaunu norimo puslapio numeri, placiau http://www.cplusplus.com/reference/string/stoi/
		this->GetJSONClientList(stoi(commandData, &pos, 0), clientSocket);
		
		return "";
	}

	if (commandData.find("connectionList") != string::npos)
	{
		// Atëjo komanda praðanti klientø sàraðo
		string data = "{ success:true, itemCount:0, items:[{ tag: 1, clientNumber : 1, connectedPort : 3389, localPort : 125476 }] }";

		return this->putHTTPheaders(data);
	}

	return "";
}

// Metodas skirtas issiusti JSON list komanda i serveri
void GClientLib::JSONapi::GetJSONClientList(int page, SOCKET clientSocket){
	// Siunciu komanda i serveri
	toServer->CommandJsonList(page, clientSocket);
}

std::string GClientLib::JSONapi::FormatJSONListACK(char* buffer, int dataSize, bool success)
{
	/* Konstruojamas pavizdys:
		{ 
			success: 'true', 
			itemCount:0, 
			items:[{ 
				id: 1, domain : 'asdasd', pcname : 'asdasd', username : 'asdasd' }]
			}
	*/

	// Nustatau irasu skaiciu
	int recordCount = dataSize / sizeof Client;
	// Nuoroda i kliento struktura
	Client* client;
	// nustatau pozicija
	int position = 0;

	std::ostringstream json;
	json << "{";
	// Visada pranesame, kad pavyko suformuoti atsaka
	json << "success:'true',";
	// Irasu skaicius
	json << "itemCount:" << recordCount << ",";
	json << "items:[";
	// Formuoju irasu masyva
	for (int i = 0; i < recordCount; i++){
		// Nustatau kliento strukturos vieta
		client = (struct Client*) &buffer[position];
		// Suvartau identifikacini numeri
		client->id = ntohl(client->id);
		// Formuoju masyvo irasa
		json << "{id:" << (int)client->id << ",domain:'" << client->domainName << "',pcname:'" << client->pcName << "',username:'" << client->userName << "'},";
		// Perstumiu pozicija
		position = position + sizeof Client;
	}
	json << "]}";

	return putHTTPheaders(json.str());
}