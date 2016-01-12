
#include "JSONapi.h"
#include <iostream>
#include <sstream>

using namespace std;

GClientLib::JSONapi::JSONapi(SettingsReader^ settings, ToServerSocket^ toServer){
	this->settings = settings;
	this->toServer = toServer;
}

// Metodas skirtas nauskaityti gauta komanda ir ja ivygdyti
std::string GClientLib::JSONapi::readCommand(string commandData)
{
	// Kintamasisi nurodantis, ad atëjo uþlausà apie klientø sàraðà
	bool clientList = false;


	// Ieskau kokia komanda atejo
	// Tikrinu ar neatejo klientu sàraðo komanda
	if (commandData.find("clientList") != string::npos)
	{
		// Atëjo komanda praðanti klientø sàraðo
		cout << "Klientu sarasas" << endl;
		std::ostringstream responseStream;
		string data = "{ success: true, itemCount : 4, items : [{ id: 1, domain : 'GMC.LOCAL', pcname : 'GMC-GEDO', username : 'gedas' }, { id: 2, domain : 'GMC.LOCAL', pcname : 'GMC-TADO', username : 'tadas' }, { id: 3, domain : 'GMC.LOCAL', pcname : 'GMC-RAMO', username : 'ramas' }, { id: 4, domain : 'GMC.LOCAL', pcname : 'GMC-ROLANDO', username : 'asdasd' }] }";
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

	if (commandData.find("connectionList") != string::npos)
	{
		// Atëjo komanda praðanti klientø sàraðo
		cout << "Sujungimu sarasas" << endl;
		std::ostringstream responseStream;
		string data = "{ success:true, itemCount:0, items:[{ tag: 1, clientNumber : 1, connectedPort : 3389, localPort : 125476 }] }";
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

	return "";
}