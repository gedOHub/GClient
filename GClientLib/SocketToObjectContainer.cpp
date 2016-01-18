#include "SocketToObjectContainer.h"

using namespace GClientLib;

using namespace cliext;
using namespace GClientLib;

GClientLib::SocketToObjectContainer::SocketToObjectContainer(void){
	this->sarasas = gcnew cliext::list<gNetSocket^>();
}


void GClientLib::SocketToObjectContainer::Add(gNetSocket^ object){
	//cout << "[SocketToObjectContainer] Prie saraso pridetas objektas. Socket: " << object->GetSocket() << " Tag:" << object->GetTag() << endl;
	this->sarasas.push_back(object);
}

gNetSocket^ GClientLib::SocketToObjectContainer::FindBySocket(int socket){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetSocket() == socket){
			//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju socket: " << socket << " rastas" << endl;
			return *i;
		}
	}
	//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju socket: " << socket << " nerastas" << endl;
	return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::FindByTag(int tag){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if (i->GetTag() == tag)
		{
			//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju tag: " << tag << " rastas" << endl;
			return *i;
		}
	}
	//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju tag: " << tag << " nerastas" << endl;
	return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::DeleteBySocket(int socket){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetSocket() == socket) {
			//gNetSocket^ tempSocket = i;
			//cout << "[SocketToObjectContainer] Is saraso pasalintas objektas. Socket: " << socket << endl;
			this->sarasas.erase(i);
			//return tempSocket;
		}
	}
	//cout << "[SocketToObjectContainer] Is saraso nepavyko pasalinti objekto. Socket: " << socket << endl;
	return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::DeleteByTag(int tag){
	cliext::list<gNetSocket^>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->GetTag() == tag && i->GetTag() != 0) {
			//gNetSocket^ tempSocket = i;
			//cout << "[SocketToObjectContainer] Is saraso pasalintas objektas. Tag: " << tag << endl;
			this->sarasas.erase(i);
			return nullptr;
			//return tempSocket;
		}
	}
	//cout << "[SocketToObjectContainer] Is saraso nepavyko pasalinti objekto. Tag: " << tag << endl;
	return nullptr;
}

