#pragma once

namespace GClientLib {
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

		virtual int Accept(SocketToObjectContainer^ container);
		virtual void Listen();
		virtual void Recive(SocketToObjectContainer^ container);
		void SetRedirectUrl(String ^url);
	};
};
