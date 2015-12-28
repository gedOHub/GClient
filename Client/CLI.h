#pragma once
#include "stdafx.h"

using namespace System;
using namespace System::Threading;
using namespace GClientLib;

ref class CLI {
	private:
		SocketToObjectContainer^ STOContainer;
		ToServerSocket^ socket;
		SettingsReader* settings;
		string ClearCLI();

	public:
		CLI(ToServerSocket^ socket, SocketToObjectContainer^ STOContainer, SettingsReader* settings);
		void Start();
};
