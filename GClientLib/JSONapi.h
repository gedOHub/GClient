#ifndef JSONAPI_H
#define JSONAPI_H

// Sisteminis includas
#include <winsock2.h>
#include <ws2tcpip.h>

#include "SettingsReader.h"
#include "ToServerSocket.h"

namespace GClientLib {
	ref class JSONapi {
		private:
			// Nustatimu skaitykle
			SettingsReader^ settings;
			// Jungtis su serveriu
			ToServerSocket^ toServer;
		public:
			JSONapi(SettingsReader^ settings, ToServerSocket^ toServer);
			// Metodas skirtas nusaityti kokia komanda gauta ir ja ivygdyti
			int readCommand(string commandData);
};
}

#endif