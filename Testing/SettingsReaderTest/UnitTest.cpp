#include "stdafx.h"
#include "GClientLib.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

using namespace GClientLib;

namespace SettingsReaderTest
{
	[TestClass]
	public ref class UnitTest
	{
	private:
		TestContext^ testContextInstance;
		static SettingsReader* settings;

	public: 
		/// <summary>
		///Gets or sets the test context which provides
		///information about and functionality for the current test run.
		///</summary>
		property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
		{
			Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
			{
				return testContextInstance;
			}
			System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
			{
				testContextInstance = value;
			}
		};

		#pragma region Additional test attributes
		//
		//You can use the following additional attributes as you write your tests:
		//
		//Use ClassInitialize to run code before running the first test in the class
		[ClassInitialize()]
		static void MyClassInitialize(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ testContext) {
			settings = new SettingsReader();
		};
		//
		//Use ClassCleanup to run code after all tests in a class have run
		[ClassCleanup()]
		static void MyClassCleanup() {
			settings->~SettingsReader();
		};
		//
		//Use TestInitialize to run code before running each test
		//[TestInitialize()]
		//void MyTestInitialize() {};
		//
		//Use TestCleanup to run code after each test has run
		//[TestCleanup()]
		//void MyTestCleanup() {};
		//
		#pragma endregion 

		// Metodas skirtas patirkinti ar nuskaite numatyta serveraddress reiksme
		[TestMethod]
		void DefaultServerAddress()
		{
			// Nustatau laukiama reiksme
			string correct = "server.jancys.net";
			// Nuskaitytos reiksmes pavadinimas
			string valueName = "serverAddress";
			// Nuskaitytos reiksmes duomenys
			string value = settings->getSetting(valueName);
			// Konvertavimas i suprantama Visual Studio formata
			String^ Correct = gcnew String(correct.c_str());
			String^ Value = gcnew String(value.c_str());
			// Lyginimas ar tokios pacios reiksmes
			Assert::AreEqual(Correct, Value);
		};

		// Metodas skirtas patirkinti ar nuskaite numatyta serverport reiksme
		[TestMethod]
		void DefaultServerPort()
		{
			// Nustatau laukiama reiksme
			string correct = "1300";
			// Nuskaitytos reiksmes pavadinimas
			string valueName = "serverPort";
			// Nuskaitytos reiksmes duomenys
			string value = settings->getSetting(valueName);
			// Konvertavimas i suprantama Visual Studio formata
			String^ Correct = gcnew String(correct.c_str());
			String^ Value = gcnew String(value.c_str());
			// Lyginimas ar tokios pacios reiksmes
			Assert::AreEqual(Correct, Value);
		};

		// Metodas skirtas patirkinti ar nuskaite numatyta serverbind reiksme
		[TestMethod]
		void DefaultServerBind()
		{
			// Nustatau laukiama reiksme
			string correct = "0.0.0.0";
			// Nuskaitytos reiksmes pavadinimas
			string valueName = "bindAddress";
			// Nuskaitytos reiksmes duomenys
			string value = settings->getSetting(valueName);
			// Konvertavimas i suprantama Visual Studio formata
			String^ Correct = gcnew String(correct.c_str());
			String^ Value = gcnew String(value.c_str());
			// Lyginimas ar tokios pacios reiksmes
			Assert::AreEqual(Correct, Value);
		};

		// Metodas skirtas patirkinti ar grazina tuscia reiksme jei klausiama nezinoma rakto
		[TestMethod]
		void DefaultNotValidValue()
		{
			// Nustatau laukiama reiksme
			string correct = "";
			// Nuskaitytos reiksmes pavadinimas
			string valueName = "sakarmakar";
			// Nuskaitytos reiksmes duomenys
			string value = settings->getSetting(valueName);
			// Konvertavimas i suprantama Visual Studio formata
			String^ Correct = gcnew String(correct.c_str());
			String^ Value = gcnew String(value.c_str());
			// Lyginimas ar tokios pacios reiksmes
			Assert::AreEqual(Correct, Value);
		};

		// Metodas skirtas patirkinti ar teisingai dirba objektas kai nëra reikalingos registø ðakos
		[TestMethod]
		void NoRegistryTree()
		{
			// Eksportuoju registru saka jei ji yra i faila
			system("reg export HKEY_CURRENT_USER\SOFTWARE\gNet gNet.reg /y");
			// Trinu nustatimu saka
			system("reg delete /f HKEY_CURRENT_USER\SOFTWARE\gNety");
			//system("reg delete HKEY_CURRENT_USER\SOFTWARE\gNet /F");

			// Sunaikinu objekta
			settings->~SettingsReader();
			
			bool tinkamasExeptionas = false;
			// Kuriu objekta is naujo
			try{
				settings = new SettingsReader();
			}
			catch (System::NullReferenceException^ a){
				tinkamasExeptionas = true;
			}
			// Grazinu jei buvo reiksmes
			system("reg import gNet.reg");
			// Tikrinu kas gavosi
			Assert::IsTrue(tinkamasExeptionas);
		};

		// Metodas skirtas patikrinti ar gerai konvertuojamas stringas
		[TestMethod]
		void StringToSTDStringTest()
		{
			// Standartinis stringas
			std::string correct = "Labas Vakaras";
			// Sisteminis stringas
			String^ string = gcnew String(correct.c_str());
			
			std::string returnString;

			// Atlieku vertima
			SettingsReader::SystemStringToStdString(string, returnString);
			// Tikrinu ar tokie patys
			Assert::AreEqual(0, correct.compare(returnString));
		};
	};
}
