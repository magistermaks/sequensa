
#include "modes.hpp"

bool file_exist( const char *path ) {
    std::ifstream infile(path);
    return infile.good();
}

std::string get_exe_path() {
	int length = wai_getExecutablePath(NULL, 0, NULL);
	char path[length + 1];
	wai_getExecutablePath(path, length, NULL);
	path[length] = '\0';
	return std::string( path );
}
