// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GClientLib.h"

using namespace GClientLib;

int _tmain(int argc, _TCHAR* argv[])
{
	// Nuskaito nustatymus is registru
	SettingsReader^ settings = gcnew SettingsReader();
	// Kuriamas visu sujungimu saraso saugykla
	SocketToObjectContainer^ STOContainer = gcnew SocketToObjectContainer();
	// Objektas augantis visu tuneliu informacija
	TunnelContainer^ tunnels = gcnew TunnelContainer();

	// 1s = 1000000 microsekundziu
	timeval time;
	time.tv_sec = 0;
	// 0.1s
	time.tv_usec = 300000;

	// --- Select funkcijos kintamieji ---
	// Inicijuoju
	fd_set skaitomiSocket, rasomiSocket, klaidingiSocket;
	// Nunulinu strukturas
	FD_ZERO(&skaitomiSocket);
	FD_ZERO(&rasomiSocket);
	// Pridedu konsole prie rasomu
	//FD_SET(0, &rasomiSocket);
	FD_ZERO(&klaidingiSocket);
	
	//Sukuria socketa jungtis prie pagrindinio serverio
	ToServerSocket^ ToServer = nullptr;
	// Tirkinu kuri protokola naudoti
	if (settings->getSetting("protocol") == "TCP"){
		ToServer = gcnew TCPToServerSocket(settings->getSetting("serverAddress"),
			settings->getSetting("serverPort"), &skaitomiSocket, &rasomiSocket, &klaidingiSocket, STOContainer, settings, tunnels);
	}
	else if (settings->getSetting("protocol") == "UDP"){
		ToServer = gcnew UDPToServerSocket(settings->getSetting("serverAddress"),
			settings->getSetting("serverPort"), &skaitomiSocket, &rasomiSocket, &klaidingiSocket, STOContainer, settings, tunnels);
	}
	else if (settings->getSetting("protocol") == "SCTP"){
		ToServer = gcnew SCTPToServerSocket(settings->getSetting("serverAddress"),
			settings->getSetting("serverPort"), &skaitomiSocket, &rasomiSocket, &klaidingiSocket, STOContainer, settings, tunnels);
	}
	else {
		printf("Gauta nezinoma protocol reiksme\n");
		exit(999);
	}
		
		
	// Tikrinam ar pavyko uzmegsti rysi iki centrinio serverio
	if(ToServer->GetSocket() == INVALID_SOCKET){
		printf("Nepavyko sukurti objekto darbui su centriniu serveriu\n");
		return 1;
	}
	// Priskiriam pradinius rezius
	Globals::maxD = ToServer->GetSocket();
	// Sukurta sujungima dedame i sarasus
	STOContainer->Add(ToServer);
	
	// Pradedu CLI gija valdymui
	CLI ^console = gcnew CLI(ToServer, STOContainer, settings);
	Thread ^consoleThread = gcnew Thread(gcnew ThreadStart(console, &CLI::Start));
	consoleThread->Start();

	// JSON API
	JSONapi^ JSON_API = gcnew JSONapi(settings, ToServer, tunnels);
	
	// TODO: padaryti kad veiktu net ir tada kai nepavyksta uzkrauti JSON socketo
	// WEB klientu socketas
	JSONapiServer^ JSON;
	try{
		// Sukuriu JSON API socket'a
		JSON = gcnew JSONapiServer(settings->getSetting("JSONapi_address"), settings->getSetting("JSONapi_port"), &skaitomiSocket, &rasomiSocket, &klaidingiSocket, JSON_API);
		// Tikrinam ar klausytis nurodytu adresu ir portu
		if (JSON->GetSocket() == INVALID_SOCKET){
			Console::WriteLine("Nepavyko atverti JSOn prievado");
			//printf("Nepavyko klausytis %s:%s\n", settings->getSetting("JSONapi_address"), settings->getSetting("JSONapi_port"));
		}
		else {
			// Nustatom maksimalu deskriptoriu
			// Tikrinu ar JSON socketas nera didensi nei ToServer :)
			if (Globals::maxD < (int)JSON->GetSocket())
				Globals::maxD = (int)JSON->GetSocket();
			// Sukurta sujungima dedame i sarasus
			STOContainer->Add(JSON);
		}
	} catch (Exception^ e) {
		Console::WriteLine("Nepavyko sukurti prievado grafinei sasajai");
		Diagnostics::Debug::WriteLine(e);
	}

	fd_set tempRead, tempWrite, tempError;	// Laikinas dekriptoriu kintamasis
	while(!Globals::quit){
		// Siam ciklui skaitomi dekriptoriai
		tempRead = skaitomiSocket;
		tempWrite = rasomiSocket;
		tempError = klaidingiSocket;
		// Pasiemam dekriptorius kurie turi kazka nuskaitimui
		if (select(Globals::maxD + 1, &tempRead, nullptr, nullptr, nullptr) < 0){
			// Select nepasiseke grazinti dekriptoriu
			Console::WriteLine(WSAGetLastError());
			switch(WSAGetLastError()){
				case WSAENOTSOCK:{
					Console::WriteLine("Socket operation on nonsocket");
				}
				default:{
					printf("Select klaida : %d\n", WSAGetLastError());
					continue;
				}
			}
		}
		// Begam per esamus sujungimus ir ieskom ar kas ka atsiunte
		for (int i = 0; i <= Globals::maxD; i++){
			
			// Tikrinam ar i-asis yra dekriptorius kuriame ivyko klaida
			if (FD_ISSET(i, &tempError)) {
				printf("Ivyko kalida %d dekriptoriuje", i);
			}
			
			/*
			// Tikrinam ar i-asis yra dekriptorius kuriame ivyko klaida
			if (FD_ISSET(i, &tempWrite)) {
				printf("Ivyko rasimas %d dekriptoriuje", i);
			}
			*/

			// Tikrinam ar i-asis yra deskriptorius is kurio reikia ka nors nuskaityti
			if (FD_ISSET(i, &tempRead)) {
				try{
					STOContainer->FindBySocket(i)->Recive(STOContainer);
				}catch(System::Exception^ e){
					//printf("Nepavyko rasti %d\n", i);
					// Salinu socket is skaitomu saraso
					FD_CLR(i, &skaitomiSocket);
					FD_CLR(i, &rasomiSocket);
					FD_CLR(i, &klaidingiSocket);
				}
			} // if (FD_ISSET(i, &read_fds)) { pabaiga
		} // for(int i = minD; i <= maxD; i++){ pabaiga
	}

	// Naikinu visus duomenis
	delete settings;
	delete STOContainer;
	delete tunnels;
	delete ToServer;

	return 0;
}
