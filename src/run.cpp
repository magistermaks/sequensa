
#include "modes.hpp"
#include "api/SeqAPI.hpp"

#define LIBLOAD_IMPLEMENT
#include "lib/libload.hpp"

std::vector<DynamicLibrary> dls;

bool load_native_lib( seq::Executor& exe, seq::FileHeader& header, std::string& path, bool v ) {

	DynamicLibrary dl( path.c_str() );
	if( dl.isLoaded() ) {

		int status = 0;

		try{
			status = dl.fetch<DynLibInit>("init")(&exe,&header);
			dls.push_back( std::move(dl) );
		}catch(...){
			status = -1;
		}

		if( status != 0 ) {
			std::cout << "Failed to load native library from: '" + path + "', error: " << status << "!" << std::endl;
			return false;
		}else if(v) {
			std::cout << "Successfully loaded native library: '" + path + "'!" << std::endl;
		}

		return true;

	}

	std::cout << "Failed to load native library from: '" + path + "', error: " << dl.getMessage() << "!" << std::endl;
	return false;

}

void unload_native_libs() {

	for( auto& lib : dls ) lib.close();

}

bool load_native_libs( seq::Executor& exe, seq::FileHeader& header, bool v ) {

	std::string path = get_exe_path();
	std::stringstream path_array( seq::util::toStdString( header.getValue("load"_b) ) );

	std::string segment;
	while( std::getline(path_array, segment, ';') ) {

		// check for lib in compiler directory
		std::string lib_path = (path + "/lib/" + segment + "/" + SEQ_LIB_NAME);

		if( file_exist( lib_path.c_str() ) ) {
			if( !load_native_lib( exe, header, lib_path, v ) ) return false;
			continue;
		}

		// check for lib in CWD
		lib_path = ("./lib/" + segment + "/" + SEQ_LIB_NAME);

		if( file_exist( lib_path.c_str() ) ) {
			if( !load_native_lib( exe, header, lib_path, v ) ) return false;
			continue;
		}

		// check for lib in program directory
		// TODO

		std::cout << "Unable to find native library: '" << segment << "'!" << std::endl;
		return false;

	}

	return true;

}

void run( std::string input, Options opt ) {

	std::ifstream infile( input );
	if( infile.good() ) {

		std::vector<seq::byte> buffer( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
		seq::ByteBuffer bb( buffer.data(), buffer.size() );
		seq::BufferReader br = bb.getReader();
		seq::FileHeader header = br.getHeader();

		if( !header.checkVersion(SEQ_API_VERSION_MAJOR, SEQ_API_VERSION_MINOR) ) {
			std::cout << "Error! Invalid Sequensa Virtual Machine version!" << std::endl;
			std::cout << "Program expected: " << header.getVersionMajor() << '.' << header.getVersionMinor() << '.' << header.getVersionPatch() << std::endl;

			// force program execution regardless of version mismatch
			if( opt.force_execution ) {
				std::cout << "Execution forced, this may cause errors!" << std::endl;
			}else{
				return;
			}

		}else{

			if( opt.verbose && !header.checkPatch(SEQ_API_VERSION_PATCH) ) {
				std::cout << "Warning! Invalid Sequensa Virtual Machine version!" << std::endl;
				std::cout << "Program expected: " << header.getVersionMajor() << '.' << header.getVersionMinor() << '.' << header.getVersionPatch() << std::endl;
			}

		}

		try{

			seq::Executor exe;

			if( !load_native_libs( exe, header, opt.verbose ) ) {
				std::cout << "Failed to create virtual environment, start aborted!" << std::endl;
				return;
			}

			if( opt.verbose ){
				std::cout << "Successfully created virtual environment, starting!" << std::endl;
			}

			exe.execute( br.getSubBuffer() );

			std::cout << "Exit value: " << exe.getResultString() << std::endl;

		}catch( seq::RuntimeError& err ) {

			std::cout << "Runtime exception!" << std::endl;
			std::cout << err.what() << (opt.verbose ? " (" + input + ")" : "") << std::endl;
			return;

		}

	}else{
		std::cout << "No such file '" << input << "' found!" << std::endl;
		std::cout << "Use '-help' for usage help." << std::endl;
		return;
	}

	unload_native_libs();
	infile.close();

}

void run( ArgParse& argp, Options opt ) {

	auto vars = argp.getValues();

	if( vars.size() == 1 ) {
		run( vars.at(0), opt );
	}else{
		std::cout << "Invalid arguments!" << std::endl;
		std::cout << "Use '-help' for usage help." << std::endl;
	}

}
