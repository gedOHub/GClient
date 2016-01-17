#ifndef ToServerSocket_H
#define ToServerSocket_H

// Sisteminiai includai
#include <iostream>

// Mano includai
#include "gNetSocket.h"
#include "ServerSocket.h"
#include "OutboundSocket.h"

namespace GClientLib {
	ref class ToServerSocket : public gNetSocket {
		private:
		char *commandBuffer;
		TagGenerator^ tag;
		SocketToObjectContainer^ STOC;
		SettingsReader^ settings;
		public:
			ToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket, SocketToObjectContainer^ STOC, SettingsReader^ settings);
		virtual int Send(char* data, int lenght) override;
		virtual void Recive(SocketToObjectContainer^ container) override;
		virtual void Connect() override;
		virtual void Reconnect() override;

		// Komandos valdymui, bendravimui su serveriu
		// LIST
		void CommandList(int page);
		void CommandListAck(int rRecv);
		void CommandHello();
		void CommandHelp();
		void CommandInitConnect(int id, int port, SocketToObjectContainer^ container);
		void CommandConnect(SocketToObjectContainer^ container);
		void CommandClear();
		void CommandBeginRead(SocketToObjectContainer^ container);
		void CommandClientConnectAck(SocketToObjectContainer^ container);
		void CommandInitConnectAck();
		// *** JSON komandos ***
		// Metodas skirtas paprasyti klientu saraso is serverio
		void CommandJsonList(int page, SOCKET socket);
		// Metodas skirtas gauta kleintu sarasa persiusti atitinkamui pbjektui apdorojimui
		void CommandJsonListAck(int rRecv, SocketToObjectContainer^ container);
		// Metodas skirtas inicijuoti sujungima su klientu
		void CommandJsonInitConnect(int id, int port, SOCKET socket);
		// Metodas apdorojantis CONNECT uzklausa
		void CommandJSONConnect(SocketToObjectContainer^ container);
		// Metodas apdodorja gauta sujungimo statusa
		void CommandJsonInitConnectAck();

		// Grazina sugeneruota zyme
		int GenerateTag();
	};
}

#endif