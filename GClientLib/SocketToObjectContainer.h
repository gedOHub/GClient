#ifndef SocketToObjectContainer_H
#define SocketToObjectContainer_H

// Sisteminiai includai
#include <cliext/utility> 
#include <cliext/list>
#include <cliext/algorithm>

#include "gNetSocket.h"

using namespace cliext;
using namespace System;

namespace GClientLib {

	// Del cross-reference problemos, placiau http://stackoverflow.com/questions/3735321/solving-cross-referencing
	ref class gNetSocket;

	ref class SocketToObjectContainer {
		private:
		// Sujungimu sarasas
		// bool, nes reikejo issvengti sutuacijos kai ServerSocket priima jungti ir perduoda TAG InboundSocket
			cliext::list<cliext::pair<bool, gNetSocket^>> sarasas;
		public:
		SocketToObjectContainer();
		// Prideda nauja pora
		void Add(gNetSocket^ object);
		// Iesko objekto pagal socket
		gNetSocket^ FindBySocket(int socket);
		// Iesko objekto pagal tag
		gNetSocket^ FindByTag(int tag);
		// Pasalina nurodita elementa pagal socket
		gNetSocket^ DeleteBySocket(int socket);
		// Pasalina nurodyta elementa pagal tag
		gNetSocket^ DeleteByTag(int tag);
		// Metodas nustato Socketa i neieskotus pagal TAG
		void SetSerchByTag(int tag, bool status);
	};
};

#endif