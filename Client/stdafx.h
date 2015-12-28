// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

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

#include <unordered_map>
#include <map>
// Reikalinga System::String i std::string
#include "msclr\marshal_cppstd.h"

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

#include "Globals.h"
#include "CLI.h"
#include "GClientLib.h"

/* KONSTANTOS */
static const int OneMBofChar = 1048576;
static const int FiveMBtoCHAR = 5242880;
static const int TenMBofChar = 10485760;

using namespace System;
using namespace System::IO;