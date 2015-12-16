#pragma once

ref class OutboundSocket: public gNetSocket
{
public:
	OutboundSocket(string, string, int, 
		fd_set* skaitomiSocket, 
		fd_set* rasomiSocket, 
		fd_set* klaidingiSocket
	);
	virtual void Connect() override;
	virtual void Recive(SocketToObjectContainer^ container) override;
};