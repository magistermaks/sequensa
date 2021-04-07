
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

#define LIBLOAD_IMPLEMENT
#include "lib/libload.hpp"

#define TRY_LOADING( path ) \
	paths.push_back( path ); \
	if( file_exist( (path).c_str() ) ) { \
		if( !load_native_lib( exe, header, (path).c_str(), verbose ) ) return false; \
		continue; \
	}

std::vector<DynamicLibrary> dls;

bool load_native_lib( seq::Executor& exe, seq::FileHeader& header, const char* path, bool v ) {

	DynamicLibrary dl( path );
	if( dl.isLoaded() ) {

		int status = 0;

		try{

			status = dl.fetch<DynLibInit>("init")(&exe,&header);
			dls.push_back( std::move(dl) );

		}catch(...){

			status = -1;

		}

		if( status != 0 ) {

			std::cout << "Failed to load native library from: '" << path << "', error: " << status << "!" << std::endl;
			return false;

		}else if(v) {

			std::cout << "Successfully loaded native library: '" << path << "'!" << std::endl;

		}

		return true;

	}

	std::cout << "Failed to load native library from: '" << path << "', error: " << dl.getMessage() << "!" << std::endl;
	return false;

}

void unload_native_libs() {

	for( auto& lib : dls ) lib.close();

}

bool load_native_libs( seq::Executor& exe, seq::FileHeader& header, bool verbose ) {

	std::string path = get_exe_path();
	path = get_directory( path );

	seq::StringTable path_table = header.getValueTable("load");

	for( const auto& segment : path_table ) {

		std::vector<std::string> paths;

		TRY_LOADING( path + "/lib/" + segment + "/" + SEQ_LIB_NAME );
		TRY_LOADING( path + "./lib/" + segment + "/" + SEQ_LIB_NAME );
#		undef TRY_LOADING

		std::cout << "Unable to find native library or module: '" << segment << "'!" << std::endl;

		if( verbose ) {

			for( std::string& path : paths ) {
				std::cout << " * Tried: '" << path << "'" << std::endl;
			}

		}

		return false;

	}

	return true;

}

void run( std::string input, Options opt ) {

	std::ifstream infile( input, std::ios::binary );
	if( infile.good() ) {

		std::vector<seq::byte> buffer( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
		seq::ByteBuffer bb( buffer.data(), buffer.size() );
		seq::BufferReader br = bb.getReader();
		seq::FileHeader header;

		try {
			header = br.getHeader();
		} catch( seq::InternalError& err ) {
			std::cout << "Error! Failed to parse file header, invalid signature!" << std::endl;
			return;
		}

		// validate versions
		if( !header.checkVersion(SEQ_API_VERSION_MAJOR, SEQ_API_VERSION_MINOR) ) {

			std::cout << "Error! Invalid Sequensa Virtual Machine version!" << std::endl;
			std::cout << "Program expected: " << header.getVersionString() << std::endl;

			// force program execution regardless of version mismatch
			if( opt.force_execution ) {
				std::cout << "Execution forced, this may cause errors!" << std::endl;
			}else{
				return;
			}

		}else{

			if( opt.verbose && !header.checkPatch(SEQ_API_VERSION_PATCH) ) {

				std::cout << "Warning! Invalid Sequensa Virtual Machine version!" << std::endl;
				std::cout << "Program expected: " << header.getVersionString() << std::endl;

			}

		}

		seq::ByteBuffer bytecode = br.getSubBuffer();

		// load string table if one is present
		seq::StringTable table = header.getValueTable("str");
		if( table.size() > 0 ) {
			bytecode.setStringTable( &table );
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

			exe.setStrictMath( opt.strict_math );

			exe.execute( bytecode );

			if( !opt.print_none ) {

				std::cout << "Exit value: ";

				if( opt.print_all ) {

					for( auto& res : exe.getResults() ) {

						std::cout << seq::util::stringCast( res ).String().getString() << " ";

					}

					std::cout << std::endl;

				}else{

					std::cout << exe.getResultString() << std::endl;

				}

			}


		}catch( seq::RuntimeError& err ) {

			std::cout << "Runtime error!" << std::endl;
			std::cout << err.what() << (opt.verbose ? " (" + input + ")" : "") << std::endl;
			return;

		}catch( seq::InternalError& err ) {

			std::cout << "Internal error!" << std::endl;
			std::cout << err.what() << (opt.verbose ? " (" + input + ")" : "") << std::endl;
			return;

		}

	}else{

		std::cout << "No such file '" << input << "' found!" << std::endl;
		return;

	}

	unload_native_libs();
	infile.close();

}

void run( ArgParse& argp, Options opt ) {

	auto vars = argp.getArgs("--run", "-r");

	if( vars.size() == 1 ) {

		run( vars.at(0), opt );

	}else{

		std::cout << "Expected one filename!" << std::endl;
		std::cout << "Use '--help' for usage help." << std::endl;

	}

}
