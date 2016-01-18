#include "SocketToSocketContainer.h"

using namespace GClientLib;

GClientLib::SocketToSocketContainer::SocketToSocketContainer(void){
	this->sarasas = gcnew cliext::list<cliext::pair<int, int>>();
}

void GClientLib::SocketToSocketContainer::Add(int clientSocket, int remoteSocket){
	cliext::pair<int, int> clientPair(clientSocket, remoteSocket);
	cliext::pair<int, int> remotePair(remoteSocket, clientSocket);
	this->sarasas.push_back(clientPair);
	this->sarasas.push_back(remotePair);
}

int GClientLib::SocketToSocketContainer::Find(int socket){
	cliext::list<cliext::pair<int, int>>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->first == socket)
		return i->second;
}
return -1;
}

void GClientLib::SocketToSocketContainer::Delete(int socket){
	cliext::list<cliext::pair<int, int>>::iterator i;
	for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
		if(i->first == socket) {
			this->sarasas.erase(i);
}
}
for(i=this->sarasas.begin(); i != this->sarasas.end(); ++i){
	if(i->second == socket) {
		this->sarasas.erase(i);
}
}
}

