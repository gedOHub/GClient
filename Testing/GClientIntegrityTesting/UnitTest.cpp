#include "stdafx.h"
#using <system.dll>
#include <windows.h> 
#include <string>
#include <iostream>
#include <cstdio>
#include <memory>

using namespace System;
using namespace System::Diagnostics;

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace std;

namespace GClientIntegrityTesting
{
	[TestClass]
	public ref class UnitTest
	{
	private:
		TestContext^ testContextInstance;
		// GClient buvimo vietos kintamasis
		static String^ process;

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
			process = gcnew String("D:\\git\\GClient\\Debug\\GClient.exe");
		};
		//
		//Use ClassCleanup to run code after all tests in a class have run
		//[ClassCleanup()]
		//static void MyClassCleanup() {};
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

		/*
			Metodas skirtas patikrinti ar kliento programine irangastartuoja
		*/
		[TestMethod]
		void IsStarting()
		{
			// ------ Proceso kurimas pasinaudojant https://msdn.microsoft.com/en-us/library/windows/desktop/ms682512(v=vs.85).aspx
			// Proceso kurimas pasinaudojant https://msdn.microsoft.com/en-us/library/system.diagnostics.process.exitcode(v=vs.110).aspx
			// Define variables to track the peak
			// memory usage of the process.
			_int64 peakPagedMem = 0, peakWorkingSet = 0, peakVirtualMem = 0;
			Process^ myProcess = nullptr;
			// Kintamasis saugnatis proceso ID;
			int pID = 0;
			try
			{
				// Start the process.
				myProcess = Process::Start(process);
				// Palaukiu puse minutes
				Sleep(500);
				// Nuskaitau proceso numeri
				pID = myProcess->Id;
			}
			finally
			{
				if (myProcess != nullptr)
				{
					myProcess->Close();
				}
			}
			// Tirkinu koks buvo proceso numeris
			Assert::AreNotEqual(pID, 0);
		};

		/*
		Metodas skirtas patikrinti ar kliento programine iranga prisjungia
		prie serverio orogramines irangos pagal numatytus parametrus

		PASTABA
		1. Reikia buti nustacius numatytus parametrus registru saugykloje
		2. Reikia buti paleidus serverio programine iranga
		*/
		[TestMethod]
		void IsConnectedToServer()
		{
			// ------ Proceso kurimas pasinaudojant https://msdn.microsoft.com/en-us/library/windows/desktop/ms682512(v=vs.85).aspx
			// Proceso kurimas pasinaudojant https://msdn.microsoft.com/en-us/library/system.diagnostics.process.exitcode(v=vs.110).aspx
			// Define variables to track the peak
			// memory usage of the process.
			_int64 peakPagedMem = 0, peakWorkingSet = 0, peakVirtualMem = 0;
			Process^ myProcess = nullptr;

			std::string result = "";

			try
			{
				// Start the process.
				myProcess = Process::Start(process);
				// Palaukiu minute
				Sleep(100000);
				// Tikrinu ar uzmegztas rysys pagal numatytus parametrus
				// Vygdau netstat komanda ir ieskau sujungimu su 1300 prievadu
				std::shared_ptr<FILE> pipe(_popen( "netstat -a -n", "r"), _pclose);
				// Nepavykus gauti isvadu grazinu nieko
				if (!pipe) cout << "Nepavyko atverti paipo" << endl;
				// Outputo buferis
				char buffer[128];
				// Skaitau buferi ir pildau stringa
				while (!feof(pipe.get())) {
					if (fgets(buffer, 128, pipe.get()) != NULL)
						result += buffer;
				}
			}
			finally
			{
				if (myProcess != nullptr)
				{
					myProcess->Close();
				}
			}
			cout << result << endl;
			// Analizuoju gautus netstat duomenis
			Assert::AreNotEqual(string::npos,result.find(":1300       ESTABLISHED"));
		};
	};
}
