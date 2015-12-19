// Komandu strukturos
// Komandu numeriai
// List - 1
enum Commands{
	HELLO = 99,	// Kompiuterio duomenu perdavimas serveriui
	LIST = 1,	// Prasimas grazinti klientu sarasa
	LIST_ACK = 11,	// Atsakas i prasima grazinti klientu sarasa
	AUTH = 13,
	AUTH_ACK = 31,
	INIT_CONNECT = 100,	// Administratoriaus iniciavimas tunelio
	INIT_CONNECT_ACK = 200,
	CONNECT = 300,	// Komanda jungtis i nurodyta prievada
	CONNECT_ACK = 400,	// Atasakas i CONNECT komanda
	CLIENT_CONNECT = 555,	// Komanda, rodanti, kad kliento prohrama prpsijnge
	CLIENT_CONNECT_ACK = 666,	// ATSAKAS i CLIENT_CONNECT_ACK
	BEGIN_READ = 777,       // Leidimas pradeti skaitytma
	BEGIN_READ_ACK = 888,   // Atsakas i BEGIN_READ
	CLOSE = 5
};

enum CONNECT_STATUS{
	INIT=1,	// Sukurtas tunelis
	CREATED = 2,	// Pavyko prisijungti prie prievado
	FAULT = 3,	// Nepavyko sukurti sujungimo kleinto puseje
};

#pragma pack(push, 1)	// Nustatom sstrukturas i tikra isdestyma

// gNet paketo antrastes struktura
struct header{
	USHORT tag;		// dydis: 2
	ULONG lenght;	// dydis: 4
};

struct APIHeader: header{
	USHORT apiNumber;	// dydis: 2
	USHORT returnTag;	// dydis: 2
};

// Kliento duomenu struktura
struct ClientInfo{
	char domainName[MAX_COMPUTERNAME_LENGTH+1];
	char pcName[MAX_COMPUTERNAME_LENGTH+1];
	char userName[MAX_COMPUTERNAME_LENGTH+1];
};

// Pilna kliento struktura
struct Client: ClientInfo{
	ULONG id;
};

struct Command{
	USHORT command;	// dydis 2
};

// HELLO
struct helloCommand : Command, ClientInfo{
};
// LIST
struct listCommand : Command{
	ULONG page;		// dydis 4
};
// LIST_ACK
struct listAckCommand: Command{
	char success;
};
// CONNECT_INIT
struct connectInitCommand: Command{
	USHORT destination_port;	// kliento prie kurio jungiames prievadas
	USHORT source_port;	// Lokalus prievadas, kuri atidarem tuneliui
	USHORT tag;		// Srauto zyme
	ULONG client_id;	// Kliento ID prie kurio jungsiuos
};
// CONNECT
struct connectCommand: Command{
	USHORT destinatio_port;	// Prievadas i kuri jungsiuos
	USHORT tag;	// Zyme, kuri bus naudojama kliento
	ULONG tunnelID;	// Tunelio ID
};
// CONNECT_ACK
struct connectAckCommand: Command{
	ULONG tunnelID;	// Tunelio ID
	USHORT status;	// Sujungimo statusas
};
// CONNECT_INIT_ACK
struct connectInitAckCommand: Command{
	USHORT status;
	ULONG client_id;
	USHORT adm_port;
	USHORT cln_port;
};
// CLIENT_CONNECT
struct clientConnectCommand : Command{
	USHORT tag;	// Srautao zyme
};
// CLIENT_CONNECT_ACK
struct clientConnectAckCommand : Command{
	USHORT tag;	// Srautao zyme
};
// BEGIN_READ
struct beginReadCommand : Command {
	USHORT tag;	// Srauto zyme
};
// BEGIN_READ_ACK
struct beginReadAckCommand : Command {
	USHORT tag;	// Srauto zyme
};


#pragma pack(pop) // nustatom i normalu isdestyma