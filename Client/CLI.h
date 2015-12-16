#pragma once
using namespace System;
using namespace System::Threading;

ref class CLI
{
private:
	SocketToObjectContainer^ STOContainer;
	ToServerSocket^ socket;
	string ClearCLI();

public:
	CLI(ToServerSocket^ socket, SocketToObjectContainer^ STOContainer);
	void Start();
};

