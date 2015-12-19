#pragma once
#include "gNetSocket.h"
ref class JSONapiServer : public gNetSocket
{
	private:
		void Bind();
		String ^redirectUrl;
	public:
		JSONapiServer(string ip, string port,
			fd_set* skaitomiSocket,
			fd_set* rasomiSocket,
			fd_set* klaidingiSocket
		);

		virtual int Accept(SocketToObjectContainer^ container) override;
		virtual void Listen() override;
		virtual void Recive(SocketToObjectContainer^ container) override;
		void SetRedirectUrl(String ^url);
};

