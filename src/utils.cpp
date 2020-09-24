
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

std::string get_cwd_path() {
	char temp[ PATH_MAX ];
	return ( getcwd(temp, sizeof(temp)) ? std::string( temp ) : std::string("") );
}

std::string get_base_name( std::string path ) {
	return path.substr( path.find_last_of("/\\") + 1 );
}

size_t get_path_hash( std::string path ) {
	return std::hash<std::string>{}( get_absolute_path( path, get_cwd_path() ) );
}

std::string get_absolute_path( std::string relative, std::string base ) {

	// normalize windows paths
	std::replace( relative.begin(), relative.end(), '\\', '/');
	std::replace( base.begin(), base.end(), '\\', '/');

	if( relative.empty() || relative.at(0) == '/' ) return relative;

	std::vector<std::string> relative_parts;
	std::vector<std::string> base_parts;

	{
		std::string part;
		std::stringstream relative_stream( relative );
		std::stringstream base_stream( base );

		while( std::getline(relative_stream, part, '/') ) relative_parts.push_back( part );
		while( std::getline(base_stream, part, '/') ) base_parts.push_back( part );
	}

	for( const auto& part : relative_parts ) {

		if( part == "." ) {
			// ignore
			continue;
		}

		if( part == ".." ) {
			base_parts.pop_back();
			continue;
		}

		base_parts.push_back(part);

	}

	std::string res;
	for( const auto& part : base_parts ) {
		res += part;
		res += '/';
	}

	if( !res.empty() ) res.pop_back();
	return res;

}
