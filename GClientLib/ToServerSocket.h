#pragma once
#include "stdafx.h"

namespace GClientLib {
	ref class ToServerSocket : public gNetSocket {
		private:
			char *commandBuffer;
			TagGenerator^ tag;
		public:
			ToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket);
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
			void CommandInitConnect(int id, int port, SocketToObjectContainer^ container, SettingsReader* settings);
			void CommandConnect(SocketToObjectContainer^ container);
			void CommandClear();
			void CommandBeginRead(SocketToObjectContainer^ container);
			void CommandClientConnectAck(SocketToObjectContainer^ container);
			void CommandInitConnectAck();
			// Grazina sugeneruota zyme
			int GenerateTag();
	};
}