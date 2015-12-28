#pragma once

namespace GClientLib {
	ref class TagGenerator {
		public:
			TagGenerator(void);
			int GetTag();
		private:
			// Esama skailiuko reiksme
			int skaitliukas;
			// Minimali reiksme
			int MIN;
			// Maksimali reiksme
			int MAX;

			void Reset();
	};
}

