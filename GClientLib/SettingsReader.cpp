
#include "SettingsReader.h"

using namespace GClientLib;

std::string GClientLib::SettingsReader::getSetting(std::string settingName){
	// Grazina nustatymo reiksme stringe
	// Grazinamos reiksmes:
	// NULL - jei nerado tokio nustatymo
	// REIKSME - jei rado

	String^ name = gcnew String(settingName.c_str());

	if (this->settings->ContainsKey(name))
	{
		std::string result;
		SystemStringToStdString(this->settings[name], result);
		return result;
	}

	return nullptr;
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
		for (int i = 0; i < subKeys->Length; i++){
			settings->Add((String^)subKeys[i], RKey->GetValue(subKeys[i])->ToString());
		}
	} catch (System::Exception^ e){
		std::cerr << "Nepavyko nuskaityti nustatimu is registru" << std::endl;
		throw e;
		return 10;
	} __finally {
		if(RKey) RKey->Close();
	}
	return 0;
}

void GClientLib::SettingsReader::SystemStringToStdString(System::String^ s, std::string& string){
	string = marshal_as<std::string>(s);
}

GClientLib::SettingsReader::SettingsReader(void)
{
	this->settings = gcnew Dictionary<String^, String^>();
	this->ReadRegistry();
}

GClientLib::SettingsReader::~SettingsReader(void)
{
}


