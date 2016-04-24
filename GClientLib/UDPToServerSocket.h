#pragma once
#include "ToServerSocket.h"
namespace GClientLib {
	ref class UDPToServerSocket :
		public ToServerSocket
	{
	public:
		UDPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
			SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel);
		// Perrasomas sujungimo metodas. Jo metu tik uzpildoma truktura, reikalinga siusti duomenims i serveri
		void virtual Connect() override;
		// Perrasau siuntimo funkcija
		virtual int Send(char* data, int lenght) override;
	protected:
		// Perrasomas socketo kurimas
		void virtual CreateSocket() override;
	private:
		// Struktura sauganti serverio adreso duomenis
		sockaddr_in* serverAddr;
	};
}

