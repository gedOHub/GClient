
#include "JSONapi.h"
#include <iostream>
#include <sstream>

using namespace std;

GClientLib::JSONapi::JSONapi(SettingsReader^ settings, ToServerSocket^ toServer, TunnelContainer^ tunnels){
	this->settings = settings;
	this->toServer = toServer;
	this->tunnels = tunnels;
}

std::string GClientLib::JSONapi::putHTTPheaders(string data)
{
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 200 OK\r\n";
	responseStream << "Access-Control-Allow-Origin: http://panel.jancys.net";
	//responseStream << this->settings->getSetting("webGUIAddress");
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
std::string GClientLib::JSONapi::readCommand(string commandData, JSONapiClient^ client)
{
	// Kintamasis skirtas nustatyti reikalinga pozicija
	size_t pos;
	// Kintamasis skirtas ieskomai daliai laikyti
	string tempString;

	// Komanudi fragmentai kuriu ieskosiu
	// Prasoma grazinti prisjungusiu klientu prie sistemos sarasa
	string clientListCommandArgument = "clientList=";
	// Komanda inicijuoja sujungima su nuroditu klientu
	string connectCommandArgument = "command=connectClient";
	// Prasoma grazinti mano inicijuotu sujungimu sarasa
	string outboundConnectionCommandArgument = "outboundConnectionList=1";
	// Prasoma grazinti inicijuotu su manimi sujungimu sarasa
	string inboundConnectionCommandArgument = "inboundConnectionList=1";

	// Ieskau kokia komanda atejo
	// * Tikrinu ar neatejo klientu sàraðo komanda
	if (commandData.find(clientListCommandArgument) != string::npos)
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
		
		// Surandu kur slepiasi clientList
		pos = commandData.find(clientListCommandArgument);
		// Salinu uzklausos pradzia iki clientList kintamojo
		commandData.erase(0, pos + clientListCommandArgument.length());
		// Likusi uzklausa: 1&start=0&limit=20 HTTP/1.1....

		// Siusiu JSON list komanda i serveri
		// Isgaunu norimo puslapio numeri, placiau http://www.cplusplus.com/reference/string/stoi/
		this->GetJSONClientList(stoi(commandData, &pos, 0), client->GetSocket());
		
		return "";
	}
	// * Tikrinu ar neatejo sujungimo komanda
	if (commandData.find(connectCommandArgument) != string::npos) 
	{
		// Uzklausos pavizdys
		//GET /?_dc=1452918791392&command=connectClient&id=5&port=12313 HTTP/1.1
		//Host: 127.0.0.1 : 3000
		//User - Agent : Mozilla / 5.0 (Windows NT 10.0; WOW64; rv:43.0) Gecko / 20100101 Firefox / 43.0
		//Accept : text / html, application / xhtml + xml, application / xml; q = 0.9, */*;q=0.8
		//Accept-Language: en-US,en;q=0.5
		//Accept-Encoding: gzip, deflate
		//X-Requested-With: XMLHttpRequest
		//Referer: http://panel.jancys.net/
		//Origin: http://panel.jancys.net
		//Connection: keep-alive

		// Nustatau ieskoma dali
		// Ieskosiu &id
		tempString = "&id=";
		// Surandu kur slepiasi id
		pos = commandData.find(tempString);
		// Trinu vska iki &id pozicijos
		commandData.erase(0, pos + tempString.length());
		// Gaunu 5&port=12313 HTTP/1.1...
		// Nuskaitu cliento numeri i integer
		int clientID = stoi(commandData, &pos, 0);
		// Nustatau ieskoma dali port=
		tempString = "&port=";
		// Salinu pradzia
		commandData.erase(0, pos + tempString.length());
		// Gaunu 12313 HTTP/1.1...
		// Nustatau prievado numeri
		int remotePort = stoi(commandData, &pos, 0);
		// Siunciu connect uzklausa i serveri
		this->ConnectClientJSON(clientID, remotePort, client->GetSocket());

		return "";
	}
	// * Tirkinu ar neatejo uzklausa del mano inicijuotu sujungimu saraso
	if (commandData.find(outboundConnectionCommandArgument) != string::npos)
	{
		// Grazinu visu mano uzmegztu sujungimu duomenis
		ReturnOutboundConnectionList(client);

		return "";
	}
	// * Tirkinu ar neatejo uzklausa del su manim inicijuotu sujungimu saraso
	if (commandData.find(inboundConnectionCommandArgument) != string::npos)
	{
		// Grazinu visu mano uzmegztu sujungimu duomenis
		ReturnInboundConnectionList(client);

		return "";
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

void GClientLib::JSONapi::ConnectClientJSON(int clientID, int portNumber, SOCKET clientSocket)
{
	toServer->CommandJsonInitConnect(clientID, portNumber, clientSocket);
}

// Metodas skirtas grazinti iseinanciu (mano inicijuotu) sujungimu sarasa JSOn formatu
void GClientLib::JSONapi::ReturnOutboundConnectionList(JSONapiClient^ client)
{
	// Formuojamo srauto buferis
	std::ostringstream json;
	// Kintamasis suagantis irasu kieki
	int recordCount = 0;
	// Kintamasis skirtas apdoroti tuneli
	Tunnel^ tunnel;

	json << "{";
	// Visada pranesame, kad pavyko suformuoti atsaka
	json << "success:'true',";
	// Pildau irasus
	json << "items:[";
	// Nustatau tuneliu saraso iteratoriu i pradzia
	tunnels->ResetIterator();
	// Kol nepasiekiau saraso pabaigos
	while (tunnels->IsIteratorAtEnd())
	{
		// Begu per visus irasus
		// Gaunu tuneli
		tunnel = tunnels->GetTunnel();

		// Tikrinu at tunelis yra mano inicijuotas tunelis
		if (tunnel->serverSocket != -1)
		{	
			// Mano inicijuotas sujungimas
			// Atspausdinu jo duomenis
			json << "{tag:" << tunnel->tag
				<< ",clientid:" << tunnel->clientid
				<< ",dport:" << tunnel->dport
				<< ",sport:" << tunnel->sport
				<< ",serverSocket:" << tunnel->serverSocket
				<< ",clientSocket:" << tunnel->clientSocket
				<< ",status:" << tunnel->status
				<< "},";

			// Pildau irasu skaitliuka
			recordCount++;
		}

		// Pereinu prie kito tunelio
		tunnels->SetIteratorToNext();
	}
	// Biagiu irasu masyva
	json << "],";
	// Pranesu kiek irasu yra
	json << "itemCount:" << recordCount << "}";

	std::string data = this->putHTTPheaders(json.str());

	// Siunciu suformuota srauta i nurodyta socketa
	client->Send(data);
}

// Metodas skirtas grazinti ateinancius (prisjungta prie amnes) sujungimu sarasa JSON formatu
void GClientLib::JSONapi::ReturnInboundConnectionList(JSONapiClient^ client)
{
	// Formuojamo srauto buferis
	std::ostringstream json;
	// Kintamasis suagantis irasu kieki
	int recordCount = 0;
	// Kintamasis skirtas apdoroti tuneli
	Tunnel^ tunnel;

	json << "{";
	// Visada pranesame, kad pavyko suformuoti atsaka
	json << "success:'true',";
	// Pildau irasus
	json << "items:[";
	// Nustatau tuneliu saraso iteratoriu i pradzia
	tunnels->ResetIterator();
	// Kol nepasiekiau saraso pabaigos
	while (tunnels->IsIteratorAtEnd())
	{
		// Begu per visus irasus
		// Gaunu tuneli
		tunnel = tunnels->GetTunnel();

		// Tikrinu at tunelis yra mano inicijuotas tunelis
		if (tunnel->serverSocket == -1)
		{
			// Mano inicijuotas sujungimas
			// Atspausdinu jo duomenis
			json << "{tag:" << tunnel->tag
				<< ",clientid:" << tunnel->clientid
				<< ",dport:" << tunnel->sport
				<< ",sport:" << tunnel->dport
				<< ",clientSocket:" << tunnel->clientSocket
				<< ",status:" << tunnel->status
				<< "},";

			// Pildau irasu skaitliuka
			recordCount++;
		}

		// Pereinu prie kito tunelio
		tunnels->SetIteratorToNext();
	}
	// Biagiu irasu masyva
	json << "],";
	// Pranesu kiek irasu yra
	json << "itemCount:" << recordCount << "}";

	std::string data = this->putHTTPheaders(json.str());

	// Siunciu suformuota srauta i nurodyta socketa
	client->Send(data);
}