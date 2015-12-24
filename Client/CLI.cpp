#include "StdAfx.h"
#include "CLI.h"


CLI::CLI(ToServerSocket^ socket, SocketToObjectContainer^ STOContainer, SettingsReader* settings){
	this->socket = socket;
	this->STOContainer = STOContainer;
	this->settings = settings;
}

void CLI::Start(){
	string zodis;
	stringstream stream;
	while(true){
		Sleep(500);
		// Isvalau ivestas komandas
		//stream.str("");
		//stream.clear();

		// gaunu visa komanda
		//getline(cin, zodis);
		// Komanda dedu streama
		//stream << zodis;
		//zodis.clear();
		// Nagrineju komanda
		//stream >> zodis;
		cin >> zodis;

		// Komanda quit ir exit
		if(zodis == "quit" || zodis == "exit"){
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
				string command;
				cin >> id;
				if( cin.fail() )
					throw "CLI_ERROR";
				cin >> port;
				if( cin.fail() )
					throw "CLI_ERROR";

				this->socket->CommandInitConnect(id, port, STOContainer, this->settings);
			}catch(System::Exception^ ){
				printf("Blogai ivesta connect komanda\n");
				zodis = this->ClearCLI();
			}
			continue;
		}

		// Jeigu atejo iki cia tada reikia kad ivede bet ka
		// Spausdinu kaip perziureti koamdnas
		printf("Ivesta neatpazinta koamnda. Komandas galima suzinoti ivedus komanda help\n");

	}	// While(zodis != "quit" || zodis != "exit")
	Globals::quit = true;
}

string CLI::ClearCLI(){
		// Isvalau ivedimo buferi
		cin.clear();
		fflush(stdin);
		return "";
}
