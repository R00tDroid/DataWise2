#pragma once

#ifdef _WIN32
	#ifdef ANALYTICSTOOLSET_EXPORTS
		#define EXPORTED  __declspec( dllexport )
	#else
		#define EXPORTED  __declspec( dllimport )
	#endif
#else
	#define EXPORTED
#endif