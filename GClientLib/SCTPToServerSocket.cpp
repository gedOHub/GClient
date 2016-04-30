#include "SCTPToServerSocket.h"


GClientLib::SCTPToServerSocket::SCTPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
	SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) : ToServerSocket(ip, port, skaitomiSocket, rasomiSocket,
	klaidingiSocket, STOC, settings, tunnel)
{
	// Nustatau pavadinima
	this->name = "SCTPToServerSocket";
}


GClientLib::SCTPToServerSocket::~SCTPToServerSocket()
{
}
