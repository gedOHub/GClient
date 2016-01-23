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

	ref class JSONapi;

	ref class JSONapiClient : public gNetSocket {
		private:
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
			
			/*
			Metodas skirtas priimti atsiunciamus duomenis is serverio, 
			perduoti JSONapi aprodojimui ir issiusti narsyklei.
				buffer- buferis, kuriame yra visa informacija
				datasize- buferyje esanciu duomenu kiekis
				success- ar gautas bent vienas irasas
			*/
			void ReciveJSONListAck(char* buffer, int dataSize, bool success);

			// Metodas skirtas issiusti duomenis i si socketa
			int GClientLib::JSONapiClient::Send(std::string buffer);

	};
};

#endif
