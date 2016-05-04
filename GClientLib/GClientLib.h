#pragma once

//#ifndef JSONapiServer_H
//#define JSONapiServer_H

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

#include <winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
//#include <WS2sctp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iphlpapi.h>
#include <stdio.h>
#include <process.h>
#include <sstream>
#include <stdlib.h>
#include <fstream>

// Pagalbiniai
#include "Structures.h"
#include "Globals.h"
#include "SettingsReader.h"
#include "SocketToObjectContainer.h"
#include "SocketToSocketContainer.h"
#include "TunnelContainer.h"
// Sujungimai
//Bazinins
#include "gNetSocket.h"
// Isvestiniai
#include "InboundSocket.h"
#include "OutboundSocket.h"
#include "ServerSocket.h"
#include "ToServerSocket.h"
#include "TCPToServerSocket.h"
#include "UDPToServerSocket.h"
#include "SCTPToServerSocket.h"
// Dirba su JSON
#include "JSONapiServer.h"
#include "JSONapiClient.h"
// JSON darbinis
#include "JSONapi.h"
//#endif