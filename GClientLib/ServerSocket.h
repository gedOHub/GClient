#ifndef ServerSocket_H
#define ServerSocket_H

// Sisteminiai includai
#include <iostream>

// Mano includai
#include "gNetSocket.h"
#include "InboundSocket.h"
#include "TunnelContainer.h"
#include "ToServerSocket.h"

namespace GClientLib {

	ref class ToServerSocket;

	ref class ServerSocket : public gNetSocket {
		private:
			void Bind();
			Tunnel^ tunnelInfo;
			ToServerSocket^ server;

		public:
			ServerSocket(string ip, int tag,
				fd_set* skaitomiSocket,
				fd_set* rasomiSocket,
				fd_set* klaidingiSocket,
				Tunnel^ tunnel,
				ToServerSocket^ server
		);
		virtual int Accept(SocketToObjectContainer^ container) override;
		virtual void Listen() override;
		virtual void Recive(SocketToObjectContainer^ container) override;
	};
};

#endif