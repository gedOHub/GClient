#pragma once
#include "ToServerSocket.h"
namespace GClientLib{
	ref class SCTPToServerSocket :
		public ToServerSocket
	{
	public:
		SCTPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
			SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel);
		virtual ~SCTPToServerSocket();
	};
}

