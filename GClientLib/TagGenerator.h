#ifndef GClientLib_H
#define GClientLib_H

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

#endif