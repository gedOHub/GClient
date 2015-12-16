#pragma once

/* Nuorodos:
http://msdn.microsoft.com/en-us/library/windows/desktop/bb530743(v=vs.85).aspx
http://msdn.microsoft.com/en-us/library/windows/desktop/ms738566(v=vs.85).aspx
*/

using namespace std;
using namespace System;
using namespace System::IO;

ref class gNetSocket
{
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
		virtual void CreateSocket();
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
		virtual SOCKET GetSocket();
		// Grazina tag
		virtual int GetTag();
		// Uzdarome sujungima
		virtual bool CloseSocket();
		// Naikiname sujungima
		virtual void ShutdownSocket();
		// Jungaimies prie nurodito prievado
		virtual void Connect(){};
		// bando inicijuoti nauja sujungima
		virtual void Reconnect(){};
		// Klausomes nurodito prievado
		virtual void Listen(){};
		// TODO: Kaip turetu buti igyvendinti sie metodai
		// Naujo kliento priemimas
		virtual int Accept(SocketToObjectContainer^ container){return -1;};
		// Duomenu siuntimas
		virtual int Send(char* data, int lenght);
		// Duomenu gavimas
		virtual void Recive(SocketToObjectContainer^ container){};
		
		virtual void SetRead(bool state);
};

