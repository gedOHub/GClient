#pragma once
#include "ToServerSocket.h"
namespace GClientLib {
	ref class TCPToServerSocket :
		public ToServerSocket
	{
	public:
		TCPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
			SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel);
	protected:
		virtual int Recive( int size ) override;
	};
}
