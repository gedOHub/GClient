#pragma once
#include "ToServerSocket.h"
#include "GClientLib.h"

namespace GClientLib{

	ref class SCTPToServerSocket :
		public ToServerSocket
	{
	public:
		SCTPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
			SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel);
		virtual ~SCTPToServerSocket();

		//Perrasomos funkcijos

		// Uzdarome sujungima
		virtual bool CloseSocket()override{ return false; }
		// Naikiname sujungima
		virtual void ShutdownSocket()override{}
		// Duomenu siuntimas
		virtual int Send(char* data, int lenght) override;
		// Metodas sksirtas priimti duomenis
		virtual int Recive( int size ) override;

	protected:
		void GetAddressInfo() override;
	private:
		
	};
}

