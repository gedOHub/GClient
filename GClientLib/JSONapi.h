#ifndef JSONAPI_H
#define JSONAPI_H

// Sisteminis includas
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Globals.h"
#include "SettingsReader.h"
#include "ToServerSocket.h"
#include "TunnelContainer.h"
#include "JSONapiClient.h"

namespace GClientLib {

	ref class JSONapiClient;

	ref class JSONapi {
		private:
			// Nustatimu skaitykle
			SettingsReader^ settings;
			// Jungtis su serveriu
			ToServerSocket^ toServer;
			// Tuneliu sarasas
			TunnelContainer^ tunnels;
			// Formuoja uzklausa i serveri gauti klientu sarasui
			void GetJSONClientList(int page, SOCKET clientSocket);
			// Formuoja uzklausa i serveri inicijuoti sujungma prie kliento
			void ConnectClientJSON(int clientID, int portNumber, SOCKET clientSocket);
			// Metodas skirtas grazinti iseinanciu (mano inicijuotu) sujungimu sarasa JSOn formatu
			void ReturnOutboundConnectionList(JSONapiClient^ clientSocket);
			// Metodas skirtas grazinti ateinancius (prisjungta prie amnes) sujungimu sarasa JSON formatu
			void ReturnInboundConnectionList(JSONapiClient^ client);
		public:
			JSONapi(SettingsReader^ settings, ToServerSocket^ toServer, TunnelContainer^ tunnels);
			// Metodas skirtas nusaityti kokia komanda gauta ir ja ivygdyti
			std::string readCommand(string commandData, JSONapiClient^ client);
			// Formuoja gautu klientu sarasa issiuntimui narsyklei
			std::string FormatJSONListACK(char* buffer, int dataSize, bool success);
			// Metodas skirtas uzdeti HTTP antrastes ant paruostu siusti JSON duomenu
			std::string putHTTPheaders(string data);
	};
};

#endif