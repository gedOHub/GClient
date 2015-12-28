#include "StdAfx.h"
#include "SettingsReader.h"

using namespace GClientLib;

std::string GClientLib::SettingsReader::getSetting(std::string settingName){
	// Grazina nustatymo reiksme stringe
	// Grazinamos reiksmes:
	// NULL - jei nerado tokio nustatymo
	// REIKSME - jie rado
	try{
		// Ieskau norimos reiksmes
		map<string, string>::const_iterator got = this->settings.find(settingName);
		// Tikrinu ka radau
		if (got == this->settings.end())
			// Neradau reiksmes
			return nullptr;
		else
			// Radus grafiname antra poros elementa
			return got->second;
	}catch(System::Exception^){
		return nullptr;
	}
}

int GClientLib::SettingsReader::ReadRegistry(){
	// Grazinomos reiksmes:
	// 0 - jei pavyko nuskaityti registrus
	// 1 - jei nepavyko nuskaityt registrus

	//Registru kintamieji
	RegistryKey^ RKey;

	//Prasom po viena registo kintamojo
	try{
		RKey = Registry::CurrentUser;
		RKey = RKey->OpenSubKey("Software");
		RKey = RKey->OpenSubKey("gNet");
		RKey = RKey->OpenSubKey("Client");
		// Info: https://msdn.microsoft.com/en-us/library/microsoft.win32.registrykey.getvaluenames(v=vs.110).aspx
		array<String^> ^ subKeys = RKey->GetValueNames();
		string tempKey;
		string tempValue;
		for (int i = 0; i < subKeys->Length; i++){
			// System::String verciam i STD ir talpinam i hash_map
			this->SystemStringToStdString(subKeys[i], tempKey);
			this->SystemStringToStdString((RKey->GetValue(subKeys[i]))->ToString(), tempValue);
			//this->settings.insert(std::make_pair(tempKey, tempValue));
			this->settings[tempKey] = tempValue;
		}
		//std::string testas = ; 
		//std::cout << testas << endl;
	} catch (System::Exception^){
		std::cerr << "Nepavyko nuskaityti nustatimu is registru" << std::endl;
		return 10;
	}
	__finally {
		if(RKey) RKey->Close();
	}
	return 0;
}

void GClientLib::SettingsReader::SystemStringToStdString(System::String^ s, std::string& string){
	string = marshal_as<std::string>(s);
}

GClientLib::SettingsReader::SettingsReader(void)
{
	this->ReadRegistry();
}

GClientLib::SettingsReader::~SettingsReader(void)
{
}
