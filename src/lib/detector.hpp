
// If build environment test failed uncomment correct line:
// #define SEQ_WINDOWS
// #define SEQ_LINUX

#ifdef SEQ_WINDOWS
#	define __SEQ_SYSTEM_CHECKED
#elif defined SEQ_LINUX
#	define __SEQ_SYSTEM_CHECKED
#endif

#ifndef __SEQ_SYSTEM_CHECKED
#	ifdef _WIN32
#		define SEQ_WINDOWS
#		define __SEQ_SYSTEM_CHECKED
#	elif defined __APPLE__
#		define SEQ_LINUX
#		define __SEQ_SYSTEM_CHECKED
#	elif defined __linux__
#		define SEQ_LINUX
#		define __SEQ_SYSTEM_CHECKED
#	elif defined __unix__
#		define SEQ_LINUX
#		define __SEQ_SYSTEM_CHECKED
#	else
#		error "Build environment test failed! Define SEQ_WINDOWS or SEQ_LINUX."
#	endif
#endif
