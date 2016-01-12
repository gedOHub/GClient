#ifndef JSONapiClient_H
#define JSONapiClient_H

// Sisteminiai includai
#include <sstream>
#include <iostream>

// Mano includai
#include "gNetSocket.h"
#include "ToServerSocket.h"
#include "JSONapi.h"

namespace GClientLib {
	ref class JSONapiClient : public gNetSocket {
		private:
			String ^redirectUrl;
			// Uzdeda JSON antraste i buferio pradzia
			int PutJSONHeader(int dataLength);
			// Siuncia nurodyta kieki duomenu  nuo buferio pradzios
			//int SendToGNetServer(SocketToObjectContainer^ container, int dataLenght);
			// Objetas organizuojantis darba su JSON
			JSONapi^ JSON;
		public:
			JSONapiClient(
				SOCKET socket,
				int tag,
				fd_set* skaitomiSocket,
				fd_set* rasomiSocket,
				fd_set* klaidingiSocket,
				JSONapi^ JSON
				);
			virtual void Recive(SocketToObjectContainer^ container) override;
			virtual void CreateSocket() override {};
};
};

#endif
