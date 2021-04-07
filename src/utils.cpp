
/*
 * MIT License
 *
 * Copyright (c) 2020 magistermaks
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "api/SeqAPI.hpp"
#include "modules.hpp"

bool load_header( seq::FileHeader* header, seq::BufferReader& br ) {
	try {
		*header = br.getHeader();
		return true;
	} catch( seq::InternalError& err ) {
		std::cout << "Error! Failed to parse file header, invalid signature!" << std::endl;
		return false;
	}
}

std::string posix_time_to_date( time_t rawtime ) {
	struct tm ts;
	char buf[255];

	ts = *gmtime(&rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", &ts);

	return std::string(buf);
}

bool validate_version( seq::FileHeader& header, bool force, bool verbose ) {

	if( !header.checkVersion(SEQ_API_VERSION_MAJOR, SEQ_API_VERSION_MINOR) ) {

		std::cout << "Error! Invalid Sequensa version!" << std::endl;
		std::cout << "Program expected: " << header.getVersionString() << std::endl;
		std::cout << "To force Sequensa to continue run again with '-f'." << std::endl;

		// continue regardless of version mismatch
		if( force ) {
			std::cout << "Sequensa forced to continue, issues may occur!" << std::endl;
		}else{
			return false;
		}

	}else{

		if( verbose && !header.checkPatch(SEQ_API_VERSION_PATCH) ) {
			std::cout << "Warning! Invalid Sequensa version!" << std::endl;
			std::cout << "Program expected: " << header.getVersionString() << std::endl;
		}

	}

	return true;
}

bool file_exist( const char *path ) {
    std::ifstream infile(path);
    return infile.good();
}

std::string get_exe_path() {
	int length = wai_getExecutablePath(NULL, 0, NULL);
	char* path = new char[length + 1];
	wai_getExecutablePath(path, length, NULL);
	path[length] = '\0';
	auto path_str = std::string(path);
	delete[] path;
	return path_str;
}

std::string get_cwd_path() {
	char temp[ CWD_MAX_PATH ];
	return ( POSIX_GETCWD(temp, sizeof(temp)) ? std::string( temp ) : std::string("") );
}

std::string get_directory( std::string& path ) {
     size_t pos = path.find_last_of("/\\");
     return (std::string::npos == pos) ? "" : path.substr(0, pos);
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

			if( !base_parts.empty() ) {
				base_parts.pop_back();
			}

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
