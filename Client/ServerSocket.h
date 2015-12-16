#pragma once

ref class ServerSocket : public gNetSocket
{
	private:
		void Bind();
		
	public:
		ServerSocket(string ip, string port, int tag, 
			fd_set* skaitomiSocket, 
			fd_set* rasomiSocket, 
			fd_set* klaidingiSocket
		);
		virtual int Accept(SocketToObjectContainer^ container) override;
		virtual void Listen() override;
		virtual void Recive(SocketToObjectContainer^ container) override;
};

