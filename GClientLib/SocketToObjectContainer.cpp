#include "SocketToObjectContainer.h"

using namespace GClientLib;

using namespace cliext;
using namespace GClientLib;

GClientLib::SocketToObjectContainer::SocketToObjectContainer(void){
	this->sarasas = gcnew cliext::list<cliext::pair<bool, gNetSocket^>>();
}


void GClientLib::SocketToObjectContainer::Add(gNetSocket^ object){
	//cout << "[SocketToObjectContainer] Prie saraso pridetas objektas. Socket: " << object->GetSocket() << " Tag:" << object->GetTag() << endl;
	cliext::pair<bool, gNetSocket^> pora(true, object);
	this->sarasas.push_back(pora);
}

gNetSocket^ GClientLib::SocketToObjectContainer::FindBySocket(int socket){
	cliext::list<cliext::pair<bool, gNetSocket^>>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if (i->second->GetSocket() == socket){
			//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju socket: " << socket << " rastas" << endl;
			return i->second;
		}
	}
	//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju socket: " << socket << " nerastas" << endl;
	return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::FindByTag(int tag){
	cliext::list<cliext::pair<bool, gNetSocket^>>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if (i->second->GetTag() == tag && i->first)
		{
			//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju tag: " << tag << " rastas" << endl;
			return i->second;
		}
	}
	//cout << "[SocketToObjectContainer] Objektas pagal paieskos kriteriuju tag: " << tag << " nerastas" << endl;
	return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::DeleteBySocket(int socket){
	cliext::list<cliext::pair<bool, gNetSocket^>>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if (i->second->GetSocket() == socket) {
			gNetSocket^ tempSocket = i->second;
			//cout << "[SocketToObjectContainer] Is saraso pasalintas objektas. Socket: " << socket << endl;
			this->sarasas.erase(i);
			return tempSocket;
		}
	}
	//cout << "[SocketToObjectContainer] Is saraso nepavyko pasalinti objekto. Socket: " << socket << endl;
	return nullptr;
}

gNetSocket^ GClientLib::SocketToObjectContainer::DeleteByTag(int tag){
	cliext::list<cliext::pair<bool, gNetSocket^>>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if (i->second->GetTag() == tag && i->second->GetTag() != 0 && i->first) {
			gNetSocket^ tempSocket = i->second;
			//cout << "[SocketToObjectContainer] Is saraso pasalintas objektas. Tag: " << tag << endl;
			this->sarasas.erase(i);
			return tempSocket;
		}
	}
	//cout << "[SocketToObjectContainer] Is saraso nepavyko pasalinti objekto. Tag: " << tag << endl;
	return nullptr;
}

void GClientLib::SocketToObjectContainer::SetSerchByTag(int tag, bool status)
{
	cliext::list<cliext::pair<bool, gNetSocket^>>::iterator i;
	for (i = this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if (i->second->GetTag() == tag && i->second->GetTag() != 0) {
			i->first = status;
		}
	}
}
