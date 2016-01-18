#ifndef INBOUNDSOCKET_H
#define INBOUNDSOCKET_H

#include "gNetSocket.h"
#include "SocketToObjectContainer.h"

namespace GClientLib {
	ref class InboundSocket : public gNetSocket {
		public:
		InboundSocket(SOCKET socket, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket);
		virtual void Recive(SocketToObjectContainer^ container) override;
		virtual void CreateSocket() override {};
	};
};

#endif