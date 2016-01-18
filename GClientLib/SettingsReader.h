#ifndef SettingsReader_H
#define SettingsReader_H

// Sisteminiai includai
#include <iostream>
#include <Windows.h>

// Mano includai

// Reikalinga System::String i std::string
#include "msclr\marshal_cppstd.h"

#using <mscorlib.dll>

using namespace System;
using namespace System::Security::Permissions;
using namespace System::Collections::Generic;
using namespace Microsoft::Win32;
// Reikalinga System::String o std::string
using namespace msclr::interop;

/*
#undef MessageBox
#define _DEFINE_DEPRECATED_HASH_CLASSES 0
*/

namespace GClientLib {
	ref class SettingsReader{
		private:
		Dictionary< String^, String^ >^ settings;
		int ReadRegistry();
		public:
		static void SystemStringToStdString(System::String^ s, std::string& string);
		std::string getSetting(std::string settingName);
		SettingsReader(void);
		~SettingsReader(void);
	};
};

#endif

