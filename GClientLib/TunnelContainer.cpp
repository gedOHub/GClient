#include "TunnelContainer.h"

using namespace GClientLib;

GClientLib::TunnelContainer::TunnelContainer(void){
	this->sarasas = gcnew cliext::list<cliext::pair<int, Tunnel^>>();
}

Tunnel^ GClientLib::TunnelContainer::Add(Tunnel^ tunelis){
	// Pildau pora
	cliext::pair<int, Tunnel^> pora( tunelis->tag, tunelis);
	// Pridedu prie saraso
	this->sarasas.push_back(pora);
	return tunelis;
}

Tunnel^ GClientLib::TunnelContainer::Add(int tag, int dport, int clientid, int sport, int serverSocket, int clientSocket)
{
	Tunnel^ tunelis;
	// Pildau Tunnel struktura
	tunelis->tag = tag;
	tunelis->dport = dport;
	tunelis->clientid = clientid;
	tunelis->sport = sport;
	tunelis->serverSocket = serverSocket;
	tunelis->clientSocket = clientSocket;
	tunelis->status = JUNGIASI;
	// Pridedu prie saraso
	return this->Add(tunelis);
}

Tunnel^ GClientLib::TunnelContainer::Find(int tag)
{
	// Apsiskelbiu itearatoriu
	cliext::list<cliext::pair<int, Tunnel^>>::iterator i;
	// Begu per visa sarasa
	for (i = this->sarasas.begin(); i != this->sarasas.end(); ++i){
		// tirkinu ar atitinka tag'as
		if (i->first == tag){
			// Radus tinkama grazinu Tunnel struktura
			return i->second;
		}
	}
	// Neradus grazinu nullptr
	return nullptr;
}

Tunnel^ GClientLib::TunnelContainer::Remove(int tag)
{
	// Nustatau iteratoriu
	cliext::list<cliext::pair<int, Tunnel^>>::iterator i;
	//Begu per visa sarasa
	for (i = this->sarasas.begin(); i != this->sarasas.end(); ++i){
		// Tirkinu ar yra toks tunelis
		if (i->first == tag) {
			// Issisaugau tunelio struktura
			Tunnel^ tunelis = i->second;
			// Trinu is saraso
			this->sarasas.erase(i);
			// Grazinu istrinto tunelio informacija
			return tunelis;
		}
	}
	return nullptr;
}

// Keicia tunelio statusa
void GClientLib::TunnelContainer::ChangeStatus(int tag, TunnelStatus status)
{
	// Nustatau iteratoriu
	cliext::list<cliext::pair<int, Tunnel^>>::iterator i;
	//Begu per visa sarasa
	for (i = this->sarasas.begin(); i != this->sarasas.end(); ++i){
		// Tirkinu ar yra toks tunelis
		if (i->first == tag) {
			// Nustatau satatusa
			i->second->status = status;
		}
	}
}