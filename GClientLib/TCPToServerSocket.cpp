#include "TCPToServerSocket.h"


GClientLib::TCPToServerSocket::TCPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, 
	fd_set* klaidingiSocket, SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) : 
	GClientLib::ToServerSocket(ip, port, skaitomiSocket, rasomiSocket, klaidingiSocket, STOC, settings, tunnel){
	this->name = "TCPToServerSocket";
}

int GClientLib::TCPToServerSocket::Recive(){
	return recv(this->Socket, &this->buffer[0], TenMBofChar, 0);
}