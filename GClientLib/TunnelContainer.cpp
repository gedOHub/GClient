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
	Tunnel^ tunelis = gcnew Tunnel();
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
	//Begu per visa sarasa
	for (i = this->sarasas.begin(); i != this->sarasas.end(); ++i){
		// Tirkinu ar yra toks tunelis
		if (i->first == tag) {
			// Nustatau satatusa
			i->second->status = status;
		}
	}
}

bool GClientLib::TunnelContainer::isEmpty()
{
	return this->sarasas.empty();
}

void GClientLib::TunnelContainer::Print()
{
	System::Console::WriteLine();
	System::Console::WriteLine("--------------------------------------------------------");
	System::Console::WriteLine("                   Sujungimu sarasas");
	System::Console::WriteLine("--------------------------------------------------------");
	// Tikrinu ar sarase yra kas nors
	if (!this->isEmpty())
	{
		// Sarasas ne tuscias
		Tunnel^ tunelis;
		//Begu per visa sarasa
		for (i = this->sarasas.begin(); i != this->sarasas.end(); ++i){
			tunelis = i->second;
			// | TAG | CLIENTID | DPORT | SPORT | SERVER_SOCKET | CLIENT_SOCKET | STATUS |
			System::Console::WriteLine("| {0} | {1} | {2} | {3} | {4} | {5} | {6} |",
				tunelis->tag, 
				tunelis->clientid, 
				tunelis->dport, 
				tunelis->sport, 
				tunelis->serverSocket, 
				tunelis->clientSocket, 
				tunelis->status);
		}
	}
	else
	{
		// Tuscias sarasas
		System::Console::WriteLine("Jus nedalyvaujate jokiame sujungime");
	}
	System::Console::WriteLine();
}

// Metodas skirtas nustatyti iteratoriu i saraso pradzia
void GClientLib::TunnelContainer::ResetIterator()
{
	i = this->sarasas.begin();
}
// Metodas skirtas patikrinti ar nepasiektas saraso galas su iteratoriumi
bool GClientLib::TunnelContainer::IsIteratorAtEnd()
{
	return (i != this->sarasas.end());
}
// Metodas skirtas gauti tunelio objekta
Tunnel^ GClientLib::TunnelContainer::GetTunnel()
{
	return i->second;
}
// Metodas skirtas perstumti iteratoriu prie kito objekto
void GClientLib::TunnelContainer::SetIteratorToNext()
{
	i++;
}