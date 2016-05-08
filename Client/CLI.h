#pragma once
#include "stdafx.h"
#include "GClientLib.h"
#include "Globals.h"

using namespace System;
using namespace System::Threading;
using namespace GClientLib;

ref class CLI {
	private:
		bool quitApp;
		SocketToObjectContainer^ STOContainer;
		ToServerSocket^ socket;
		SettingsReader^ settings;
		std::string ClearCLI();
		

	public:
		CLI(GClientLib::ToServerSocket^ socket, GClientLib::SocketToObjectContainer^ STOContainer, GClientLib::SettingsReader^ settings);
		void Start();
		bool getQuitStatus();
};
