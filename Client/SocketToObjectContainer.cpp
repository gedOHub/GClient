#include "StdAfx.h"
#include "SocketToObjectContainer.h"


SocketToObjectContainer::SocketToObjectContainer(void){
	this->sarasas = gcnew cliext::list<gNetSocket^>();
}


void SocketToObjectContainer::Add(gNetSocket^ object){
	this->sarasas.push_back(object);
}

gNetSocket^ SocketToObjectContainer::FindBySocket(int socket){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetSocket() == socket){
			return *i;
		}
	}
	return nullptr;
}

gNetSocket^ SocketToObjectContainer::FindByTag(int tag){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetTag() == tag)
			return *i;
	}
	return nullptr;
}

gNetSocket^ SocketToObjectContainer::DeleteBySocket(int socket){
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

gNetSocket^ SocketToObjectContainer::DeleteByTag(int tag){
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