#pragma once

namespace GClientLib {
	ref class OutboundSocket : public gNetSocket
	{
	public:
		OutboundSocket(string, string, int,
			fd_set* skaitomiSocket,
			fd_set* rasomiSocket,
			fd_set* klaidingiSocket
			);
		virtual void Connect();
		virtual void Recive(SocketToObjectContainer^ container);
	};
}