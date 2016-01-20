#ifndef OutboundSocket_H
#define OutboundSocket_H

#include "ToServerSocket.h"
#include "gNetSocket.h"

namespace GClientLib {

	ref class ToServerSocket;

	ref class OutboundSocket : public gNetSocket {
		private:
			ToServerSocket^ server;
		public:
		OutboundSocket(string, string, int,
			fd_set* skaitomiSocket,
			fd_set* rasomiSocket,
			fd_set* klaidingiSocket,
			ToServerSocket^ server
		);
		virtual void Connect() override;
		virtual void Recive(SocketToObjectContainer^ container) override;
	};
};

#endif