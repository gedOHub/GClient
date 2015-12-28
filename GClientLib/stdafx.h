// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

/**
Sis failas atsakingas uz .lib failo formavima
*/

#define GCLIENTLIB_EXPORTS

#ifdef GCLIENTLIB_EXPORTS
#define GCLIENTLIB_API __declspec(dllexport) 
#else
#define GCLIENTLIB_API __declspec(dllimport) 
#endif

#include "targetver.h"

#include <msclr/auto_gcroot.h>
#include <string>
#include <tchar.h>
#include <iostream>

// TODO: reference additional headers your program requires here
// Pridedamos bibliotekos
// TODO: Interpti mscorlib 
// Adresas: http://msdn.microsoft.com/en-us/library/microsoft.win32.registrykey.getsubkeynames.aspx

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma comment (lib, "Ws2_32.lib")
#using <mscorlib.dll>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <cliext/utility> 
#include <cliext/list>
#include <cliext/algorithm>
#include <process.h>
#include <sstream>
#include <stdlib.h>
#include <fstream>

/*
Includai kad veiktu visuose projektuose kuriuose bus naudojama
*/

#include "Globals.h"
#include "Structures.h"
#include "gNetSocket.h"
#include "SocketToObjectContainer.h"
#include "SettingsReader.h"
#include "OutboundSocket.h"
#include "InboundSocket.h"
#include "ServerSocket.h"
#include "TagGenerator.h"
#include "ToServerSocket.h"
#include "JSONapiServer.h"
#include "JSONapiClient.h"

using namespace System;
using namespace System::IO;