#include "ToServerSocket.h"
#include "JSONapiClient.h"

using namespace GClientLib;

GClientLib::ToServerSocket::ToServerSocket(string ip, string port, fd_set* skaitomiSocket, fd_set* rasomiSocket, fd_set* klaidingiSocket, SocketToObjectContainer^ STOC, SettingsReader^ settings, TunnelContainer^ tunnel) : GClientLib::gNetSocket(ip, port, 0, skaitomiSocket, rasomiSocket, klaidingiSocket){
	// Nustatom, kad galetu tiek siusti tiek gauti duomenis
	this->read = true;
	this->write = true;
	this->name = "ToServerSocket";
	this->STOC = STOC;
	this->settings = settings;
	this->tunnels = tunnel;

	this->Connect();
	this->tag = gcnew TagGenerator(STOC);
	this->commandBuffer = new char[OneMBofChar];
	this->CommandHello(); // Siunciam labas serveriui
	this->CommandHelp(); // Isspausdinam galimas komandas
}

int GClientLib::ToServerSocket::Send(char* data, int lenght){
	int rSend = 0;
	while(rSend != lenght){
		rSend = rSend + send(this->Socket, &data[rSend], lenght, 0);
}
return rSend;
}

void GClientLib::ToServerSocket::Connect(){
	// Begam per visus gautus rezultatus ir bandom jungtis
	for(struct addrinfo * ptr = this->addrResult; ptr != NULL; ptr = ptr->ai_next){
		// Pasirienkam rezultata ir bandom jungtis
		int rConnect = connect(this->Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		//cout << "Sujungimas grazino: " << rConnect << endl;
		// Tikrinam ar pavyko prijsungti
		if(rConnect == SOCKET_ERROR) {
			// Bandom tol, kol pavyks, delsiant viena minute
			printf("Klaida jungiantis i %s:%s\n", this->IP->c_str(), this->PORT->c_str());
			cout << "Klaida jungiantis i " << this->IP << ":" << this->PORT << " Kodas: " << WSAGetLastError() << endl;
		} else break;
	}
}

void GClientLib::ToServerSocket::Reconnect(){
	cout << "Reconnect" << endl;
	// Uzdarau senaja jungti
	this->CloseSocket();
	// Sunaikinu WSA
	WSADATA wsaData;
	int wdaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (wdaResult != 0) {
		printf("WSAStartup failed: %d\n", wdaResult);
		// Nepavykus sunaikiname kintamaji
		return;
}
this->Connect();
}

void GClientLib::ToServerSocket::Recive(SocketToObjectContainer^ container){
	using namespace std;
	// Bandau gauti duoemis
	// Nusiskaitau paketo antraste
	int head_dydis = sizeof header;
	int rRecv = recv(this->Socket, &buffer[0], head_dydis, 0);
	if(rRecv == 0){ // Klientas uzdare sujungima
		// Uzdarau sujungima
		printf("Serveris uzdare sujungima :(\n");
		// Bandau jungti sprie serverio is naujo
		this->Reconnect();
	} else if( rRecv < 0 ){ // Sujungime ivyko klaida
		printf("Klaida sujungime %d\n", this->Socket);
	} else { // Gauti duomenis, persiusiu i serveri
		//printf("Gauta duomenu %d\n", rRecv);

		this->head = (struct header*) &this->buffer[0];
		// Atstatau zyme ir ilgi i tinkama pavidala
		this->head->tag = ntohs(this->head->tag);
		this->head->lenght = ntohl(this->head->lenght);

		// Nusiskaitau pilna paketa
		//cout << "[" << this->name << "]Laukiama " << this->head->lenght << endl;
		int paketo_ilgis = this->head->lenght+sizeof(header);
		while( rRecv != paketo_ilgis ){
			rRecv = rRecv + recv(this->Socket, &buffer[rRecv], paketo_ilgis - rRecv, 0);
			//cout << "[" << this->name << "]Siuo metu turiu " << rRecv << endl;
		}
		//cout << "[" << this->name << "]Gavau " << rRecv << endl;

		switch(this->head->tag){
			// Atejo komanda is serverio
			case 0: {
				// Nustatinesiu kokia komanda atejo
				Command* cmd = (struct Command*) &this->buffer[sizeof header];
				// Verciu komanda i tikraji pavidala
				cmd->command = ntohs(cmd->command);

				// Tikrinu komanda
				switch (cmd->command){
						// LIST_ACK komanda
					case LIST_ACK:{
						//cout << "[" << this->name << "] Gavau LIST_ACK" << endl;
						this->CommandListAck(rRecv);
						break;
					}
					case JSON_LIST_ACK:{
						//cout << "[" << this->name << "] Gavau JSON_LIST_ACK" << endl;
						this->CommandJsonListAck(rRecv, container);
						break;
					}
					case CONNECT:{
						//cout << "[" << this->name << "] Gavau CONNECT" << endl;
						this->CommandConnect(container);
						break;
					}
					case JSON_CONNECT: {
						// Inicijuojamas sujungimas su nurodytu prievadu
						//cout << "[" << this->name << "] Gavau JSON_CONNECT" << endl;
						this->CommandJSONConnect(container);
						break;
					}
					case INIT_CONNECT_ACK:{
						//cout << "[" << this->name << "] Gavau INIT_CONNECT_ACK" << endl;
						this->CommandInitConnectAck();
						break;
					}
					case JSON_INIT_CONNECT_ACK:{
						//cout << "[" << this->name << "] Gavau JSON_INIT_CONNECT_ACK" << endl;
						this->CommandJsonInitConnectAck();
						break;
					}
					case BEGIN_READ:{
						//cout << "[" << this->name << "] Gavau BEGIN_READ" << endl;
						this->CommandBeginRead(container);
						break;
					}
					case CLIENT_CONNECT_ACK:{
						//cout << "[" << this->name << "] Gavau CLIENT_CONNECT_ACK" << endl;
						this->CommandClientConnectAck(container);
						break;
					}
					case CLOSE_TUNNEL:{
						//cout << "[" << this->name << "] Gavau CLOSE_TUNNEL" << endl;
						// Suvartau duomenis
						closeTunnelCommand* close = (struct closeTunnelCommand*) &this->buffer[sizeof header];
						close->tag = ntohs(close->tag);
						this->CloseTunnel(close->tag);
						break;
					}
				}// Baigiu switch(cmd->command){
				break;
			}
			// Atejo duomenys, kuriuos reikia permesti i kita sokceta
			default: {
				// Persiunciu duomenis i  reikalinga socket
				// Pildau buferi
				int rSend = container->FindByTag(this->head->tag)->Send(&this->buffer[sizeof(header)], this->head->lenght);
				break;
			}
		} // switch(this->head->tag)
	} // if(rRecv == 0)
}

void GClientLib::ToServerSocket::CommandList(int page){
	// Nustatau head i buferio pradzia
	header* head = (struct header*) &this->commandBuffer[0];

	// Pildau antraste
	head->tag = htons(Globals::CommandTag);
	head->lenght = htonl(sizeof (listCommand) );

	// Kuriu komandos struktura
	listCommand* list = (struct listCommand*) &this->commandBuffer[sizeof(header)];
	// Pildau duomenis
	list->command = htons(LIST);
	list->page = htonl(page);
	// cout << ntohl(list->page) << endl;
	//list->page = page;

	// Siunciu uzklausa i serveri
	int size = sizeof header + sizeof listCommand;
	int rSend = this->Send(this->commandBuffer, size);
}

void GClientLib::ToServerSocket::CommandJsonList(int page, SOCKET destinationSocket){
	// Nustatau head i buferio pradzia
	header* head = (struct header*) &this->commandBuffer[0];

	// Pildau antraste
	head->tag = htons(Globals::CommandTag);
	head->lenght = htonl(sizeof(jsonListCommand));

	// Kuriu komandos struktura
	jsonListCommand* list = (struct jsonListCommand*) &this->commandBuffer[sizeof(header)];
	// Pildau duomenis
	// Komandos numeris
	list->command = htons(JSON_LIST);
	// Norimas puslapis
	list->page = htonl(page);
	// Gristantcio socketo numeris
	list->socketID = htonl(destinationSocket);

	// Siunciu uzklausa i serveri
	int size = sizeof header + sizeof jsonListCommand;
	int rSend = this->Send(this->commandBuffer, size);
}

void GClientLib::ToServerSocket::CommandListAck(int rRecv){
	// Nustatau ar pavyko gauti klientu sarasa
	listAckCommand* list = (struct listAckCommand*) &this->buffer[ sizeof header ];
	if( list->success ){

		// Spausidnu klientu duomenis
		Client* client;
		int position = sizeof header + sizeof listAckCommand;
		printf("-----------------------------------------------------------------------\n" );
		printf("| %10s | %16s | %16s | %16s |\n", "ID", "Sritis", "Kompiuteris", " Naudotojas " );
		printf("-----------------------------------------------------------------------\n" );
		for( int i = 0; position <= (rRecv-1); i++ ){
			client = (struct Client*) &this->buffer[ position ];
			client->id = ntohl( client->id );
			printf( "| %10d | %16s | %16s | %16s |\n", client->id, client->domainName, client->pcName, client->userName );
			position = position + sizeof Client;
}
printf("-----------------------------------------------------------------------\n" );

} else {
	// Negrazino klientu sarasa
	printf( "Gautas tuscias klientu sarasas\n" );
}
}

void GClientLib::ToServerSocket::CommandJsonListAck(int rRecv, SocketToObjectContainer^ container){
	// Nuskaitau paketo tipa
	jsonListAckCommand* list = (struct jsonListAckCommand*) &this->buffer[sizeof header];
	// Suvartau socketID kintamaji
	list->socketID = ntohl(list->socketID);
	try {
		// Ieskau reikalingo socketo
		JSONapiClient^ client = (JSONapiClient^)container->FindBySocket(list->socketID);
		// Perduodu gautus duomenis tolimesniam apdorojimui
		client->ReciveJSONListAck(&this->buffer[sizeof header + sizeof jsonListAckCommand], rRecv - sizeof header + sizeof jsonListAckCommand, list->success);
	} catch (Exception^ e) {
		//cout << "Klaida konvertuojant i JSONapiCLient" << endl;
		Console::WriteLine(e->Message);
	}
}

void GClientLib::ToServerSocket::CommandHello(){
	string buffer;
	header* head = (struct header* ) &this->commandBuffer[0];
	// Pildau antraste
	// Nustatua taga
	head->tag = htons(Globals::CommandTag);
	head->lenght = htonl(sizeof helloCommand);

	helloCommand* hello = (struct helloCommand* ) &this->commandBuffer[ sizeof header ];
	// Pildau perduodamus duomenis
	// Komandos numeris
	hello->command = htons(HELLO);
	// Nustatau srities varda
	buffer = marshal_as<std::string>(System::Environment::UserDomainName);
	// Kopijuoju sriteis varda
	memcpy( &hello->domainName[0], buffer.c_str(), sizeof( hello->domainName ) );
	// Nustatau kompiuterio varda
	buffer = marshal_as<std::string>(System::Environment::MachineName);
	// Kopijuoju kompiuteiro varda
	memcpy( &hello->pcName[0], buffer.c_str(), sizeof( hello->pcName ) );
	// Nustatau naudotojo varda
	buffer = marshal_as<std::string>(System::Environment::UserName);
	// Kopijuoju naudotojo varda
	memcpy( &hello->userName[0], buffer.c_str(), sizeof( hello->userName ) );

	// Siunciu hello paketa
	this->Send( this->commandBuffer, sizeof( header ) + sizeof ( helloCommand ) );
}

void GClientLib::ToServerSocket::CommandInitConnect(int id, int port, SocketToObjectContainer^ container){
	// Formuoju tunelio struktura
	Tunnel^ tunnel = gcnew Tunnel();
	// Nustatau zinoma tunelio inforamcija
	// Nutolusio kliento prievadas
	tunnel->dport = port;
	// Kliento, prie kurio jungiamasi, ID
	tunnel->clientid = id;

	// Formuoju komanda connect
	int newTag = tag->GetTag();
	cout << "Suformuojamas naujas srautas " << newTag << endl;
	// Nustatau tunelio zyme
	tunnel->tag = newTag;

	// Inicijuoju listen socketa
	ServerSocket^ newSocket = gcnew ServerSocket(settings->getSetting("bindAddress"), newTag, this->skaitomi, this->rasomi, this->klaidingi, tunnel, this);
	// Nustatau serverSocket
	tunnel->serverSocket = newSocket->GetSocket();
	// Nustatau atverta preivada
	tunnel->sport = newSocket->GetPort();

	if(newSocket->GetSocket() == SOCKET_ERROR){
		printf("Nepavyko sukurti besiklausancio prievado");
		delete newSocket;
		return;
	} else {
		container->Add(newSocket);

		if(Globals::maxD < (int)newSocket->GetSocket())
		Globals::maxD = newSocket->GetSocket();

		// Pildau InitCommand komandos pakeita
		connectInitCommand* connect = (struct connectInitCommand* ) &this->commandBuffer[sizeof(header)];
		// Nurodau komanda
		connect->command = htons(INIT_CONNECT);
		// Nurodau savo atverta prievada
		connect->source_port = htons(newSocket->GetPort());
		// Nurodau srauto zyme
		connect->tag = htons(newTag);
		// Nurodau koki prievada noriu pasiekti
		connect->destination_port = htons(port);
		// Nurodau klienta
		connect->client_id = htonl(id);
		// Pildau headeri
		this->head = (struct header*) &this->commandBuffer[0];
		head->tag = htons(0);
		head->lenght = htonl(sizeof connectInitCommand);
		// Siunciu komanda i serveri
		this->Send(this->commandBuffer, sizeof(header)+sizeof(connectInitCommand));
		printf("Laukiu atsakymo is serverio...\n");
		
		// Pridedu tuneli prie tuneliu saraso
		tunnels->Add(tunnel);
	}
};

void GClientLib::ToServerSocket::CommandJsonInitConnect(int id, int port, SOCKET socket)
{
	// Formuoju tunelio struktura
	Tunnel^ tunnel = gcnew Tunnel();
	// Nustatau zinoma tunelio inforamcija
	// Nutolusio kliento prievadas
	tunnel->dport = port;
	// Kliento, prie kurio jungiamasi, ID
	tunnel->clientid = id;

	// Formuoju komanda connect
	int newTag = tag->GetTag();
	cout << "Suformuojamas naujas srautas " << newTag << endl;
	// Nustatau tunelio zyme
	tunnel->tag = newTag;

	// Inicijuoju listen socketa
	ServerSocket^ newSocket = gcnew ServerSocket(settings->getSetting("bindAddress"), newTag, this->skaitomi, this->rasomi, this->klaidingi, tunnel, this);
	// Nustatau serverSocket
	tunnel->serverSocket = newSocket->GetSocket();
	// Nustatau atverta preivada
	tunnel->sport = newSocket->GetPort();

	if (newSocket->GetSocket() == SOCKET_ERROR){
		printf("[JSON]Nepavyko sukurti besiklausancio prievado");
		delete newSocket;
		return;
	}
	else {
		STOC->Add(newSocket);

		if (Globals::maxD < (int)newSocket->GetSocket())
			Globals::maxD = newSocket->GetSocket();

		// Pildau InitCommand komandos pakeita
		jsonConnectInitCommand* connect = (struct jsonConnectInitCommand*) &this->commandBuffer[sizeof(header)];
		// Nurodau komanda
		connect->command = htons(JSON_INIT_CONNECT);
		// Nurodau savo atverta prievada
		connect->source_port = htons(newSocket->GetPort());
		// Nurodau srauto zyme
		connect->tag = htons(newTag);
		// Nurodau koki prievada noriu pasiekti
		connect->destination_port = htons(port);
		// Nurodau klienta
		connect->client_id = id;
		// Nurodau klienta
		connect->client_id = htonl(connect->client_id);
		// Nurodau JSON socketa
		connect->socketID = htonl(socket);
		// Pildau headeri
		this->head = (struct header*) &this->commandBuffer[0];
		head->tag = htons(0);
		head->lenght = htonl(sizeof jsonConnectInitCommand);

		// Siunciu komanda i serveri
		this->Send(this->commandBuffer, sizeof(header) + sizeof(jsonConnectInitCommand));
		printf("[JSON]Laukiu atsakymo is serverio...\n");

		// Pridedu tuneli prie tuneliu saraso
		tunnels->Add(tunnel);
	}
}

void GClientLib::ToServerSocket::CommandConnect(SocketToObjectContainer^ container){
	connectCommand* connect = (struct connectCommand*) &this->buffer[ sizeof header ];
	// Nustatau duomenis i tinkama puse
	connect->destinatio_port = ntohs(connect->destinatio_port);
	connect->source_port = ntohs(connect->source_port);
	connect->tag = ntohs(connect->tag);
	connect->tunnelID = ntohl(connect->tunnelID);
	connect->client_id = ntohl(connect->client_id);
	// Generuoju tag sujungimui
	int status = INIT;
	// Kuriu sujungima
	char portas[20];
	_itoa_s(connect->destinatio_port, portas, 10);
	OutboundSocket^ connectSocket = gcnew OutboundSocket(settings->getSetting("bindAddress"), portas, connect->tag, this->skaitomi, this->rasomi, this->klaidingi, this);

	if(Globals::maxD < (int)connectSocket->GetSocket())
	Globals::maxD = connectSocket->GetSocket();

	if( connectSocket->GetSocket() == SOCKET_ERROR ){
		// Jei nepavyko sukurti sujungimo
		cout << "Nepavyko prisijungti prie " << connect->destinatio_port << endl;
		status = FAULT;
		delete connectSocket;
	} else {
		// Sukurus sujungima
		status = CREATED;
		container->Add(connectSocket);
		// Pildau tunelio informacija
		//int tag, int dport, int clientid, int sport, int serverSocket, int clientSocket
		this->tunnels->Add(connect->tag, connect->destinatio_port, connect->client_id, connect->source_port, INVALID_SOCKET, connectSocket->GetSocket());
		// Nustatau statusa i laukia programos
		this->tunnels->ChangeStatus(connect->tag, LAUKIA_PROGRAMOS);
	}
	// CONNECT_ACK
	// Formuoju atsaka serveriui
	// Formuoju connect ack paketa
	connectAckCommand* connectAck = (struct connectAckCommand*) &this->commandBuffer[sizeof(header)];
	connectAck->command = htons(CONNECT_ACK);
	connectAck->tunnelID = htonl(connect->tunnelID);
	connectAck->status = htons(status);
	// Formuoju gNet paketa
	this->head = (struct header*) &this->commandBuffer[0];
	head->tag = htons(0);
	head->lenght = htonl(sizeof(connectAckCommand));
	// Siunciu serveriui
	this->Send( this->commandBuffer, sizeof(header)+sizeof(connectAckCommand) );
}

void GClientLib::ToServerSocket::CommandJSONConnect(SocketToObjectContainer^ container)
{
	jsonConnectCommand* connect = (struct jsonConnectCommand*) &this->buffer[sizeof header];
	// Nustatau duomenis i tinkama puse
	connect->destinatio_port = ntohs(connect->destinatio_port);
	connect->source_port = ntohs(connect->source_port);
	connect->tag = ntohs(connect->tag);
	connect->tunnelID = ntohl(connect->tunnelID);
	// Generuoju tag sujungimui
	int status = INIT;
	// Kuriu sujungima
	char portas[20];
	_itoa_s(connect->destinatio_port, portas, 10);
	OutboundSocket^ connectSocket = gcnew OutboundSocket(settings->getSetting("bindAddress"), portas, connect->tag, this->skaitomi, this->rasomi, this->klaidingi, this);

	if (Globals::maxD < (int)connectSocket->GetSocket())
		Globals::maxD = connectSocket->GetSocket();

	if (connectSocket->GetSocket() == SOCKET_ERROR){
		// Jei nepavyko sukurti sujungimo
		cout << "Nepavyko prisijungti prie " << connect->destinatio_port << endl;
		status = FAULT;
		delete connectSocket;
	}
	else {
		// Sukurus sujungima
		status = CREATED;
		container->Add(connectSocket);
	}
	// CONNECT_ACK
	// Formuoju atsaka serveriui
	// Formuoju connect ack paketa
	jsonConnectAckCommand* connectAck = (struct jsonConnectAckCommand*) &this->commandBuffer[sizeof(header)];
	connectAck->command = htons(JSON_CONNECT_ACK);
	connectAck->tunnelID = htonl(connect->tunnelID);
	connectAck->status = htons(status);
	// socketID nevartytas, persiunciu is karto suvartyta
	connectAck->socketID = connect->socketID;
	// Formuoju gNet paketa
	this->head = (struct header*) &this->commandBuffer[0];
	head->tag = htons(0);
	head->lenght = htonl(sizeof(jsonConnectAckCommand));
	// Siunciu serveriui
	this->Send(this->commandBuffer, sizeof(header) + sizeof(jsonConnectAckCommand));
}

void GClientLib::ToServerSocket::CommandInitConnectAck(){
	connectInitAckCommand* ack = (struct connectInitAckCommand*) &this->buffer[sizeof(header)];
	// Suvartau tinkama tvarka skacius
	ack->status = ntohs(ack->status);
	ack->client_id = ntohl(ack->client_id);
	ack->adm_port = ntohs(ack->adm_port);
	ack->adm_tag = ntohs(ack->adm_tag);
	ack->cln_port = ntohs(ack->cln_port);

	// Spausdinu rezultatus
	switch(ack->status){
		// Nepavykus uzmegzti rysio
		case INIT: {
			printf("Sujungimas per ilgai inicijuojamas, bandykite kurti sujungima is naujo\n");
			// Salinu tuneli is tuneliu saraso
			// TODO: Surasti koki TAG grazino
			Tunnel^ removedTunnel = tunnels->Remove(ack->adm_tag);
			break;
		}
		// Pavykus uzmegzti rysi
		case CREATED:{
			long int client_id = ack->client_id, adm_port = ack->adm_port, cln_port = ack->cln_port;
			cout << "Sujungimas sekmingai sukurtas su " << client_id << " klientu" << endl;
			cout << "Pas Jus atverta " << adm_port << " jungtis, kuri sujungta su kliento " << cln_port << " jungtimi" << endl;
			cout << "Jungimos duomenys: " << settings->getSetting("bindAddress") << ":" << adm_port << endl;
			// Nustatau tunelio statusa i laukia programos
			tunnels->ChangeStatus(ack->adm_tag, LAUKIA_PROGRAMOS);
			break;
		}
		// Nepavykus uzmegzti rysio
		case FAULT:{
			printf("Nepavyko sukurti sujungimo\n");
			// Salinu tuneli is tuneliu saraso
			Tunnel^ removedTunnel = tunnels->Remove(ack->adm_tag);
			break;
		}
	}
}

void GClientLib::ToServerSocket::CommandJsonInitConnectAck(){
	jsonConnectInitAckCommand* ack = (struct jsonConnectInitAckCommand*) &this->buffer[sizeof(header)];
	// Suvartau tinkama tvarka skacius
	ack->status = ntohs(ack->status);
	ack->client_id = ntohl(ack->client_id);
	ack->adm_port = ntohs(ack->adm_port);
	ack->adm_tag = ntohs(ack->adm_tag);
	ack->cln_port = ntohs(ack->cln_port);

	// Spausdinu rezultataus
	switch (ack->status){
	case INIT: {
		printf("Sujungimas per ilgai inicijuojamas, bandykite kurti sujungima is naujo\n");
		Tunnel^ removedTunnel = tunnels->Remove(ack->adm_tag);
		break;
	}
	case CREATED:{
		long int client_id = ack->client_id, adm_port = ack->adm_port, cln_port = ack->cln_port;
		cout << "Sujungimas sekmingai sukurtas su " << client_id << " klientu" << endl;
		cout << "Pas Jus atverta " << adm_port << " prievadas, kuri sujungta su kliento " << cln_port << " prievadu" << endl;
		cout << "Jungimos duomenys: " << settings->getSetting("bindAddress") << ":" << adm_port << endl;
		tunnels->ChangeStatus(ack->adm_tag, LAUKIA_PROGRAMOS);
		break;
	}
	case FAULT:{
		printf("Nepavyko sukurti sujungimo\n");
		Tunnel^ removedTunnel = tunnels->Remove(ack->adm_tag);
		break;
	}
	}
}

void GClientLib::ToServerSocket::CommandBeginRead(SocketToObjectContainer^ container){
	beginReadCommand* read = (struct beginReadCommand*) &this->buffer[sizeof header];
	// Suvaratau dautus duomensi i geraja puse
	read->tag = ntohs(read->tag);

	// Ieskau socketo pagal zyma
	gNetSocket^ socket = container->FindByTag(read->tag);
	socket->SetRead(true);

	// Siunciu atsaka i serveri
	// Pildau BEGIN_READ_ACK paketa
	beginReadAckCommand* readAck = (struct beginReadAckCommand*) &this->commandBuffer[ sizeof(header) ];
	readAck->command = htons(BEGIN_READ_ACK);
	readAck->tag = htons(read->tag);
	// Pildau antraste
	this->head = (struct header*) &this->commandBuffer[0];
	head->tag = htons(Globals::CommandTag);
	head->lenght = htonl(sizeof beginReadAckCommand );

	//cout << "[" << this->name << "] Siunciu BEGIN_READ_ACK" << endl;
	this->Send(&this->commandBuffer[0], sizeof header + sizeof beginReadAckCommand);

	// Nustatua tunelio status ai vyskta komunikacija
	this->tunnels->ChangeStatus(read->tag, KOMUNIKACIJA);
}

void GClientLib::ToServerSocket::CommandClientConnectAck(SocketToObjectContainer^ container){
	beginReadAckCommand* ack = (struct beginReadAckCommand*) &this->buffer[sizeof(header)];
	// Nustatau tag i tinkama puse
	ack->tag = ntohs(ack->tag);

	// Ieskau nustatymo pagal zyme
	gNetSocket^ socket = container->FindByTag(ack->tag);
	// Ijungiu skaityma
	socket->SetRead(true);
	// KOMUNIAKCIJA PRASIDEDA :)

	// Nustatua tuneli i komunikacijos statusa
	this->tunnels->ChangeStatus(ack->tag, KOMUNIKACIJA);
}

std::string GClientLib::ToServerSocket::CommandCloseTunnel(int tag)
{
	// Nustatau head i buferio pradzia
	header* head = (struct header*) &this->commandBuffer[0];

	// Pildau antraste
	head->tag = htons(Globals::CommandTag);
	head->lenght = htonl(sizeof(closeTunnelCommand));

	// Kuriu komandos struktura
	closeTunnelCommand* close = (struct closeTunnelCommand*) &this->commandBuffer[sizeof(header)];
	// Pildau duomenis
	close->tag = htons(tag);
	close->command = htons(CLOSE_TUNNEL);

	// Siunciu uzklausa i serveri
	int size = sizeof header + sizeof closeTunnelCommand;
	int rSend = this->Send(this->commandBuffer, size);

	return this->CloseTunnel(tag);
}

std::string GClientLib::ToServerSocket::CloseTunnel(int tag)
{
	// Tikrinu ar toks tunelis egzistuoja
	if (this->tunnels->Find(tag))
	{
		// Jei radau

		// Salinu is tuneliu saraso
		Tunnel^ tunelis = this->tunnels->Remove(tag);
		// Tirkinu is kurios puses inicijuojamas nutraukimas
		if (tunelis->serverSocket == -1)
		{
			// Is tos prie kurios jungesi, nutolusios

			// Naikinu kliento objekta
			OutboundSocket^ client = (OutboundSocket^)STOC->FindBySocket(tunelis->clientSocket);
			if (client != nullptr)
			{
				// Salinu is saraso
				STOC->DeleteBySocket(client->GetSocket());
				// naikinu objekta
				delete client;
			}
		}
		else
		{
			// Jei nutraukiama is iniciatoriaus puses

			// Naikinu kliento objekta
			InboundSocket^ client = (InboundSocket^)STOC->FindBySocket(tunelis->clientSocket);
			if (client != nullptr)
			{
				// Salinu is saraso
				STOC->DeleteBySocket(client->GetSocket());
				// naikinu objekta
				delete client;
			}

			// Naikinu serverio objekta
			ServerSocket^ server = (ServerSocket^)STOC->FindBySocket(tunelis->serverSocket);
			if (client != nullptr)
			{
				// Salinu is saraso
				STOC->DeleteBySocket(server->GetSocket());
				// naikinu objekta
				delete server;
			}
		}

		return "Sujungimas nr. " + std::to_string(tag) + " sekmingai nutrauktas";
	}
	return "Sujungimas nr. " + std::to_string(tag) + "nerastas";
}

void GClientLib::ToServerSocket::CommandHelp(){
	printf(" ---------------------------------------------------------------------------\n");
	printf("    gNet klientas\n");
	printf(" ---------------------------------------------------------------------------\n");
	printf(" Galimos komandos:\n");
	printf("   list $page - praso klientu sarano nuroditos saraso dalies\n");
	printf("       $page - pusalpio nuemris, kuri norime gauti. I puslapi telpa 20 irasu\n");
	printf("   connect $id $destination-port - inicijuoja sujungima su klientu\n");
	printf("       $id - kliento id sistemoje. Galima perziureti list komanda\n");
	printf("       $destination-port - kliento prievadas i kuri bus norima jungtis\n");
	printf("   connection \n");
	printf("       print - isveda informacija apie sujungimus\n");
	printf("       close $id - uzdaromas sujungimas\n");
	printf("           $id - uzdaromo sujungimo numeris. Suzinimas su connection print komanda\n");
	printf("   help - isspausdina sia pagalba\n");
	printf("   clear - isvalo komandini langa ir atspausdina pagalba\n");
	printf("   exit - baigia kliento darba\n");
	printf("   quit - ziureti exit\n");
	printf(" ---------------------------------------------------------------------------\n");
}

void GClientLib::ToServerSocket::CommandClear(){
	System::Console::Clear();
	this->CommandHelp();
}

int GClientLib::ToServerSocket::GenerateTag(){
	return this->tag->GetTag();
}

void GClientLib::ToServerSocket::PrintTunnelList()
{
	// Saukiu tuneliu saraso spausdinima
	this->tunnels->Print();
}
