#ifdef GCLIENTLIB_EXPORTS
	#define GCLIENTLIB_API __declspec(dllexport) 
#else
	#define GCLIENTLIB_API __declspec(dllimport) 
#endif