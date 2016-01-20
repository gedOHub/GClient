#ifndef SocketToSocketContainer_H
#define SocketToSocketContainer_H

// Sisteminiai includai
#include <cliext/utility> 
#include <cliext/list>
#include <cliext/algorithm>

// Mano includai

namespace GClientLib {
	ref class SocketToSocketContainer {
		private:
			// Sujungimu sarasas
			cliext::list<cliext::pair<int, int>> sarasas;
		public:
			SocketToSocketContainer(void);
			void Add(int, int);
			int Find(int);
			void Delete(int);
	};
};

#endif