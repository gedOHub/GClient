#ifndef GClientLib_H
#define GClientLib_H

#include "SocketToObjectContainer.h"
#include "gNetSocket.h"

namespace GClientLib {

	ref class SocketToObjectContainer;
	ref class gNetSocket;

	ref class TagGenerator {
	private:
		// Nuroda i socketu-objektu saugykla
		SocketToObjectContainer^ container;
		// Metodas skirtas patikrinti ar tag'as yra naudojamas
		bool isFree(int tag);
	public:
		TagGenerator(GClientLib::SocketToObjectContainer^ con);
		int GetTag();
		private:
		// Esama skailiuko reiksme
		int skaitliukas;
		// Minimali reiksme
		int MIN;
		// Maksimali reiksme
		int MAX;

		void Reset();
	};
};

#endif