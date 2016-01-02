#include "stdafx.h"

#include "GClientLib.h"
#include "Globals.h"
#include <winsock2.h>

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace System::Diagnostics;
using namespace GClientLib;

namespace ServerSocketTest
{
	[TestClass]
	public ref class UnitTest
	{
	private:
		TestContext^ testContextInstance;

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
		//[ClassInitialize()]
		//static void MyClassInitialize(TestContext^ testContext) {};
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

		[TestMethod]
		void IsServerSocketListenning()
		{
			// --- Select funkcijos kintamieji ---
			// Inicijuoju
			fd_set skaitomiSocket, rasomiSocket, klaidingiSocket;
			// Nunulinu strukturas
			FD_ZERO(&skaitomiSocket);
			FD_ZERO(&rasomiSocket);
			FD_ZERO(&klaidingiSocket);
			// Sukuriu ServerSocket tipo kintamaji
			ServerSocket^ server = gcnew ServerSocket("localhost", 0, &skaitomiSocket, &rasomiSocket, &klaidingiSocket);
			// Tarpiniai kintamieji
			int error_code;
			int error_code_size = sizeof(error_code);
			// Tikrinu ar prievadu yra klausomasi
			// Placiau https://msdn.microsoft.com/en-us/library/windows/desktop/ms738544(v=vs.85).aspx
			Assert::AreEqual(0, getsockopt(server->GetSocket(), SOL_SOCKET, SO_ACCEPTCONN, (char*)&error_code, &error_code_size));
		};
	};
}
