#pragma once
#pragma managed

#include "Stdafx.h"

/* Nuorodos:
http://msdn.microsoft.com/en-us/library/windows/desktop/bb530743(v=vs.85).aspx
http://msdn.microsoft.com/en-us/library/windows/desktop/ms738566(v=vs.85).aspx
*/

using namespace std;
using namespace System;
using namespace System::IO;

namespace GClientLib {

	// Del cross-reference problemos, placiau http://stackoverflow.com/questions/3735321/solving-cross-referencing
	ref class SocketToObjectContainer;

	ref class gNetSocket {
		protected:
			fd_set* skaitomi;
			fd_set* rasomi;
			fd_set* klaidingi;
			string *IP;
			string *PORT;
			SOCKET Socket;
			addrinfo *addrResult, *addrHints;
			int TAG;
			char* buffer;
			header* head;
			bool read, write;
			char* name;
			// ---Funkcijos---
			// Gaunam galimus adresu varaintus
			void GetAddressInfo();
			void CreateSocket();
		public:
			// Konstruktorius
			gNetSocket(string ip, string port, int tag,
				fd_set* skaitomiSocket,
				fd_set* rasomiSocket,
				fd_set* klaidingiSocket
				);
			gNetSocket(int socket, int tag,
				fd_set* skaitomiSocket,
				fd_set* rasomiSocket,
				fd_set* klaidingiSocket
				);
			// Destruktorius
			~gNetSocket();
			// Graziname socket
			SOCKET GetSocket();
			// Grazina tag
			int GetTag();
			// Grazina kokiu preivadu klausosi
			int GetPort();
			// Uzdarome sujungima
			bool CloseSocket();
			// Naikiname sujungima
			void ShutdownSocket();
			// Jungaimies prie nurodito prievado
			void Connect();
			// bando inicijuoti nauja sujungima
			void Reconnect();
			// Klausomes nurodito prievado
			void Listen();
			// TODO: Kaip turetu buti igyvendinti sie metodai
			// Naujo kliento priemimas
			int Accept(SocketToObjectContainer^ container);
			// Duomenu siuntimas
			int Send(char* data, int lenght);
			// Duomenu gavimas
			void Recive(SocketToObjectContainer^ container);

			void SetRead(bool state);
			// Grazina varda
			char* GetName();
	};
}