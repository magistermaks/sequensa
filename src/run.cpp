
#include "modes.hpp"

#define LIBLOAD_IMPLEMENT
#define LIBLOAD_LINUX
#include "lib/libload.hpp"

std::vector<DynamicLibrary> dls;

bool file_exist( const char *fileName ) {
    std::ifstream infile(fileName);
    return infile.good();
}

std::string get_exe_path() {
	int length = wai_getExecutablePath(NULL, 0, NULL);
	char path[length + 1];
	wai_getExecutablePath(path, length, NULL);
	path[length] = '\0';

	return std::string( path );
}

bool load_native_lib( seq::Executor& exe, seq::FileHeader& header, std::string& path, bool v ) {

	DynamicLibrary dl( path.c_str() );
	if( dl.isLoaded() ) {
		return dl.fetch<DynLibInit>("init")(&exe,&header);
	}

	std::cout << "Unable to load native library from: " + path + "!" << std::endl;
	std::cout << "Error: " << dl.getMessage() << std::endl;
	return false;

}

bool load_native_libs( seq::Executor& exe, seq::FileHeader& header, bool v ) {

	std::string path = get_exe_path();
	std::stringstream path_array( seq::util::toStdString( header.getValue("load"_b) ) );

	std::string segment;
	while( std::getline(path_array, segment, ';') ) {

		std::string lib_path = (path + "/lib/" + segment + "/" + SEQ_LIB_NAME);

		if( file_exist( lib_path.c_str() ) ) {
			if( !load_native_lib( exe, header, lib_path, v ) ) {
				std::cout << "Library '" << segment << "' failed to load!" << std::endl;
				return false;
			}else{
				if( v ) std::cout << "Loaded native library: '" << segment << "' from: " << lib_path << std::endl;
			}
			continue;
		}

		lib_path = ("./lib/" + segment + "/" + SEQ_LIB_NAME);

		if( file_exist( lib_path.c_str() ) ) {
			if( !load_native_lib( exe, header, lib_path, v ) ) {
				std::cout << "Library '" << segment << "' failed to load!" << std::endl;
				return false;
			}else{
				if( v ) std::cout << "Loaded native library: '" << segment << "' from: " << lib_path << std::endl;
			}
			continue;
		}

	}

	return true;

}

void run( std::string input, bool v ) {

	std::ifstream infile( input );
	if( infile.good() ) {

		std::vector<seq::byte> buffer( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
		seq::ByteBuffer bb( buffer.data(), buffer.size() );
		seq::BufferReader br = bb.getReader();
		seq::FileHeader header = br.getHeader();

		try{

			seq::Executor exe;
			if( !load_native_libs( exe, header, v ) ) {
				std::cout << "Unable to create virtual environment! Stopping!" << std::endl;
				return;
			}else{
				if( v ) std::cout << "Successfully created virtual environment! Starting!" << std::endl;
			}
			exe.execute( br.getSubBuffer() );

			std::cout << "Exit value: " << seq::util::toStdString(seq::util::stringCast(exe.getResult()).String().getString()) << std::endl;

		}catch( seq::RuntimeError& err ) {

			std::cout << "Runtime exception!" << std::endl;
			std::cout << err.what() << (v ? " (" + input + ")" : "") << std::endl;
			return;

		}

	}else{
		std::cout << "No such file '" << input << "' found!" << std::endl;
		std::cout << "Use '-help' for usage help." << std::endl;
		return;
	}

	infile.close();

}

void run( ArgParse& argp ) {

	auto vars = argp.getValues();

	if( vars.size() == 1 ) {
		run( vars.at(0), argp.hasFlag( "v" ) );
	}else{
		std::cout << "Invalid arguments!" << std::endl;
		std::cout << "Use '-help' for usage help." << std::endl;
	}

}
