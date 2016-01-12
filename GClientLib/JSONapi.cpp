
#include "JSONapi.h"

GClientLib::JSONapi::JSONapi(SettingsReader^ settings, ToServerSocket^ toServer){
	this->settings = settings;
	this->toServer = toServer;
}

// Metodas skirtas nauskaityti gauta komanda ir ja ivygdyti
int GClientLib::JSONapi::readCommand(string commandData)
{
	// Ieskau kokia komanda atejo

}