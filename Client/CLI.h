#pragma once
#include "stdafx.h"
#include "GClientLib.h"

using namespace System;
using namespace System::Threading;
using namespace GClientLib;

ref class CLI {
	private:
		SocketToObjectContainer^ STOContainer;
		ToServerSocket^ socket;
		SettingsReader^ settings;
		std::string ClearCLI();

	public:
		CLI(GClientLib::ToServerSocket^ socket, GClientLib::SocketToObjectContainer^ STOContainer, GClientLib::SettingsReader^ settings);
		void Start();
};
