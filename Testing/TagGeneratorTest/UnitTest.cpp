#include "stdafx.h"

#include "GClientLib.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace GClientLib;

namespace TagGeneratorTest
{
	[TestClass]
	public ref class UnitTest
	{
	private:
		TestContext^ testContextInstance;
		static TagGenerator^ generator;
		

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
			generator = gcnew TagGenerator();
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

		// Metodas skirtas patirkinti ar gautas tinkamas tipas
		[TestMethod]
		void GetTagTest()
		{
			// Gaunu pirma zyme
			// Pirma zyme turi buti 1001
			int corrent = 1001;
			// Tikrinu ar tokia reiksme
			Assert::AreEqual(corrent, generator->GetTag());
		};

		// Metodas skirtas nustatyti ar skaitliukas nustatomas is naujo
		[TestMethod]
		void ResetTest()
		{
			// Kintamasis del tipo gavimo
			int expected = 1001; 
			// Generuoju skaitliuko darba
			//  64533, nes 65534 - ( 1001 + 1, nes jau viena tag'a gavau ankstesniu testu) 
			for (int i = 0; i < 64533; i++){
				generator->GetTag();
			}
			// Tirkinu ar skitliukas pradejo skaiciuoti is pradziu
			Assert::AreEqual(expected, generator->GetTag());
		};
	};
}
