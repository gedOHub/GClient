#ifndef ServerSocket_H
#define ServerSocket_H

// Sisteminiai includai
#include <iostream>

// Mano includai
#include "gNetSocket.h"
#include "InboundSocket.h"

namespace GClientLib {
	ref class ServerSocket : public gNetSocket {
		private:
		void Bind();

		public:
		ServerSocket(string ip, int tag,
			fd_set* skaitomiSocket,
			fd_set* rasomiSocket,
			fd_set* klaidingiSocket
		);
		virtual int Accept(SocketToObjectContainer^ container) override;
		virtual void Listen() override;
		virtual void Recive(SocketToObjectContainer^ container) override;
	};
};

#endif