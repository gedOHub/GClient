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
		// Jungaimies prie nurodito prievado
		//virtual void Connect()override{}
		// bando inicijuoti nauja sujungima
		virtual void Reconnect()override{}
		// Klausomes nurodito prievado
		virtual void Listen()override{}
		// TODO: Kaip turetu buti igyvendinti sie metodai
		// Naujo kliento priemimas
		virtual int Accept(SocketToObjectContainer^ container)override{ return -1; };
		// Duomenu siuntimas
		virtual int Send(char* data, int lenght) override;
		// Duomenu gavimas
		virtual void Recive(SocketToObjectContainer^ container)override{}



	protected:
		// Sukuriamas socketas
		virtual void CreateSocket()override;
	private:
	};
}

