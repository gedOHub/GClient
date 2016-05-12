#pragma once
#include "ToServerSocket.h"
#include <process.h>

using namespace System;
using namespace System::Threading;

namespace GClientLib {
	ref class UDPToServerSocket :
		public ToServerSocket
	{
	public:
		UDPToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket,
			SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel);

		~UDPToServerSocket();
		// Perrasomas sujungimo metodas. Jo metu tik uzpildoma truktura, reikalinga siusti duomenims i serveri
		void virtual Connect() override;
		// Perrasau siuntimo funkcija
		virtual int Send(char* data, int lenght) override;
		// Grazina i koki porta bus jungiamasi
		virtual int GetPort() override;
	protected:
		// Perrasomas socketo kurimas
		void virtual CreateSocket() override;
		// Metodas sksirtas priimti duomenis
		virtual int Recive(int size) override;
	private:
		// Kintamasis skisrtas saugoti serverio adreso duomenis
		struct sockaddr* serverAddress;
		// Kintamasis skirtas valdyti siuntimo blokavimui
		bool locked = false;
		// Kintamasis skirtas zmeti rysio uztikrinimo gyvai darba
		bool live = true;
		// Kintamasiss kirtas saugoti nuoroda i gija
		ThreadStart^ ackThreadDelegate;
		Thread^ ackThread;

		// Metodas skirtas siusti keep alive paketus
		void SendKeepAlive();
		// Metodas skirtas uzrakinti siuntimui
		bool LockSending();
		// Metodas skirtas atrakinti siuntimui
		bool UnlockSending();
		// Metodas skirtas sukurti nuajai gyjai ir rpadeti jos darbui
		void StartAckThread();
		// Metodas skirtas sustabdyti gyja ir baigti jos darba
		void StopAckThread();
	};
}

