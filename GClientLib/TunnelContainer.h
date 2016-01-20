#ifndef TunnelContainer_H
#define TunnelContainer_H

// Sisteminiai includai
#include <cliext/utility> 
#include <cliext/list>
#include <cliext/algorithm>

namespace GClientLib {

	// Tunelio statusai
	enum TunnelStatus
	{
		JUNGIASI = 1,	//Uzmezgamas rysys tarp klientu
		LAUKIA_PROGRAMOS = 2,	// Laukia kol prisijungs norima kliento porgramine iranga
		KOMUNIKACIJA = 3	// Tuneliu vyksta komunikacija
	};

	// Struktura kuri bus naudojama tunelio infoamcijai saugoti
	ref struct Tunnel
	{
		int tag;		//Tunelio zyme
		int dport;		//Prievadas, prie kurio jungesi
		int clientid;	//Kliento ID su kuriuo sujungta
		int sport;		//Vietinis prievadas
		int serverSocket;	//Socketas, prie kuris priima duomenu srauta
		int clientSocket;	//Socketas, kuris prisjunge prie laukaincio serverio socketo
		int status;		// Sujungimo statusas (Jungiasi, prisjungta, laukia jungties)
	};

	ref class TunnelContainer {
	private:
		// Tuneliu sarasas
		cliext::list<cliext::pair<int, Tunnel^>> sarasas;
	public:
		// Konstruktorius
		TunnelContainer();
		// Pridedamas naujas tunelis. PERRASO statusa i JUNGIAMASI
		Tunnel^ Add(Tunnel^ tunelis);
		// Pridedamas naujas tunelis. Statusa nustato i JUNGIAMASI
		Tunnel^ Add(int tag, int dport, int clientid, int sport, int serverSocket, int clientSocket);
		// Tunelio paieska pagal tag
		Tunnel^ Find(int tag);
		// Salina tuneli pagal tag
		Tunnel^ Remove(int tag);
		// Keicia tunelio statusa
		void ChangeStatus(int tag, TunnelStatus status);
		// Metodas tikrina ar sarasas yra tuscias
		bool isEmpty();
		// Metodas, kuris i CMD langa issausdina ka turi
		void Print();
	};
};

#endif