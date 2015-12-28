#pragma once
#include "stdafx.h"

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
			virtual int Accept(SocketToObjectContainer^ container);
			virtual void Listen();
			virtual void Recive(SocketToObjectContainer^ container);
	};
}
