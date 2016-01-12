#ifndef OutboundSocket_H
#define OutboundSocket_H

#include "gNetSocket.h"

namespace GClientLib {
	ref class OutboundSocket : public gNetSocket {
		public:
		OutboundSocket(string, string, int,
			fd_set* skaitomiSocket,
			fd_set* rasomiSocket,
			fd_set* klaidingiSocket
		);
		virtual void Connect() override;
		virtual void Recive(SocketToObjectContainer^ container) override;
	};
};

#endif