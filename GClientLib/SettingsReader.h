#pragma once

#include "Stdafx.h"

#include <map>
// Reikalinga System::String i std::string
#include "msclr\marshal_cppstd.h"

#using <mscorlib.dll>

using namespace System;
using namespace System::Security::Permissions;
using namespace Microsoft::Win32;
using namespace stdext;
// Reikalinga System::String o std::string
using namespace msclr::interop;

/*
#undef MessageBox
#define _DEFINE_DEPRECATED_HASH_CLASSES 0
*/

namespace GClientLib {
	class SettingsReader{
		private:
			map<string, string> settings;
			int ReadRegistry();
		public:
			static void SystemStringToStdString(System::String^ s, std::string& string);
			std::string getSetting(std::string settingName);
			SettingsReader(void);
			~SettingsReader(void);
	};
}
