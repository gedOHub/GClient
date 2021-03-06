#include "StdAfx.h"
#include "CLI.h"

using namespace GClientLib;

CLI::CLI(ToServerSocket^ socket, SocketToObjectContainer^ STOContainer, SettingsReader^ settings){
	this->socket = socket;
	this->STOContainer = STOContainer;
	this->settings = settings;

}

void CLI::Start(){
	string zodis;
	stringstream stream;
	while(true){
		Sleep(500);

		cin >> zodis;

		// Komanda quit ir exit
		if(zodis == "quit" || zodis == "exit"){
			this->quitApp = true;
			break;
		}

		// Komanda list
		// Norima gauti visu prisjungusiuju sarasa
		if(zodis == "list"){
			try{
				// Puslapio numerio kintamasis
				short int page;
                cin >> page;
				if( cin.fail() )
					throw "CLI_ERROR";

				// Siunciama uzklausa serveriui
				// Kad grazinti visu prisijungusiuju sarasa
				this->socket->CommandList(page);
			}catch(System::Exception^){
				printf("Blogai ivesta list komanda\n");
				zodis = this->ClearCLI();
			}
			continue;
		}

		if(zodis == "help"){
			this->socket->CommandHelp();
			continue;
		}

		if(zodis == "clear"){
			this->socket->CommandClear();
			continue;
		}

		if(zodis == "connect"){
			try{
				int id, port;
				cin >> id;
				if( cin.fail() )
					throw "CLI_ERROR";
				cin >> port;
				if( cin.fail() )
					throw "CLI_ERROR";

				this->socket->CommandInitConnect(id, port, STOContainer );
			}catch(System::Exception^ ){
				printf("Blogai ivesta connect komanda\n");
				zodis = this->ClearCLI();
			}
			continue;
		}

		if (zodis == "connection")
		{
			try{
				cin >> zodis;
				if (zodis == "print")
				{
					this->socket->PrintTunnelList();
					continue;
				}
				if (zodis == "close")
				{
					int tagNr;
					cin >> tagNr;
					cout << this->socket->CommandCloseTunnel(tagNr) << endl;
					continue;
				}
			}
			catch (System::Exception^){
				printf("Blogai ivesta connect komanda\n");
				zodis = this->ClearCLI();
			}
		}

		// Jeigu atejo iki cia tada reikia kad ivede bet ka
		// Spausdinu kaip perziureti koamdnas
		printf("Ivesta neatpazinta koamnda. Komandas galima suzinoti ivedus komanda help\n");

	}	// While(zodis != "quit" || zodis != "exit")
}

string CLI::ClearCLI(){
		// Isvalau ivedimo buferi
		cin.clear();
		fflush(stdin);
		return "";
}

bool CLI::getQuitStatus(){
	// Isvalau ivedimo buferi
	return this->quitApp;
}
