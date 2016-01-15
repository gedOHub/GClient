#ifndef JSONAPI_H
#define JSONAPI_H

// Sisteminis includas
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Globals.h"
#include "SettingsReader.h"
#include "ToServerSocket.h"

namespace GClientLib {
	ref class JSONapi {
		private:
			// Nustatimu skaitykle
			SettingsReader^ settings;
			// Jungtis su serveriu
			ToServerSocket^ toServer;
			// Metodas skirtas uzdeti HTTP antrastes ant paruostu siusti JSON duomenu
			std::string putHTTPheaders(string data);
		public:
			JSONapi(SettingsReader^ settings, ToServerSocket^ toServer);
			// Metodas skirtas nusaityti kokia komanda gauta ir ja ivygdyti
			std::string readCommand(string commandData, SOCKET clientSocket);
			// Formuoja uzklausa i serveri gauti klientu sarasui
			void GetJSONClientList(int page, SOCKET clientSocket);
			// Formuoja gautu klientu sarasa issiuntimui narsyklei
			std::string FormatJSONListACK(char* buffer, int dataSize, bool success);
};
}

#endif