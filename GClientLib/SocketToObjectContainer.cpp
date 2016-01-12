#include "SocketToObjectContainer.h"

using namespace GClientLib;

using namespace cliext;
using namespace GClientLib;

GClientLib::SocketToObjectContainer::SocketToObjectContainer(void){
	this->sarasas = gcnew cliext::list<gNetSocket^>();
}


void GClientLib::SocketToObjectContainer::Add(gNetSocket^ object){
	this->sarasas.push_back(object);
}

gNetSocket^ GClientLib::SocketToObjectContainer::FindBySocket(int socket){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetSocket() == socket){
			return *i;
}
}
return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::FindByTag(int tag){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetTag() == tag)
		return *i;
}
return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::DeleteBySocket(int socket){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetSocket() == socket) {
			//gNetSocket^ tempSocket = i;
			this->sarasas.erase(i);
			//return tempSocket;
}
}
return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::DeleteByTag(int tag){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetTag() == tag && i->GetTag() != 0) {
			//gNetSocket^ tempSocket = i;
			this->sarasas.erase(i);
			return nullptr;
			//return tempSocket;
}
}
return nullptr;
}

