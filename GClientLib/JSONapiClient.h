#pragma once

namespace GClientLib {
	ref class JSONapiClient : public gNetSocket {
	private:
		String ^redirectUrl;
		// Uzdeda JSON antraste i buferio pradzia
		int PutJSONHeader(int dataLength);
		// Siuncia nurodyta kieki duomenu  nuo buferio pradzios
		int SendToGNetServer(SocketToObjectContainer^ container, int dataLenght);
	public:
		JSONapiClient(SOCKET socket, int tag, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket);
		virtual void Recive(SocketToObjectContainer^ container);
		virtual void CreateSocket();
		void SetRedirectUrl(String ^url);
	};
};

