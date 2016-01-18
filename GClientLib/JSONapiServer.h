#pragma once

#ifndef JSONapiServer_H
#define JSONapiServer_H

#include "gNetSocket.h"
#include "ToServerSocket.h"
#include "JSONapi.h"
#include "JSONapiClient.h"

namespace GClientLib {

	ref class JSONapiServer : public gNetSocket {
		private:
			// Objektas organizuojantis darba su JSON
			JSONapi^ json;

			// Metodas pradedantis kalusytis prisijungimu
			void Bind();
		public:
			JSONapiServer(string ip, string port,
				fd_set* skaitomiSocket,
				fd_set* rasomiSocket,
				fd_set* klaidingiSocket,
				JSONapi^ JSON
		);

		virtual int Accept(SocketToObjectContainer^ container) override;
		virtual void Listen() override;
		virtual void Recive(SocketToObjectContainer^ container) override;
	};
};

#endif