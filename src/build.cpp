
/*
 * MIT License
 *
 * Copyright (c) 2020, 2021 magistermaks
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

bool failed = false;
bool singular = false;
bool silent = false;

bool build( std::string input, std::vector<seq::byte>* buffer, std::vector<std::string>* dependencies, std::vector<std::string>* natives, bool verbose, seq::Compiler& compiler ) {

	std::ifstream infile( input );
	if( infile.good() ) {

		std::vector<std::string> headerData;
		std::string base = get_directory( input );
		compiler.setLoadTable( &headerData );

		try{

			// The extra brackets are needed or things break, because C++
			std::string content( (std::istreambuf_iterator<char>(infile)), (std::istreambuf_iterator<char>()) );
			*buffer = compiler.compile(content);

		}catch( seq::CompilerError& err ){}

		// set by error handle
		if( failed ) {

			std::cout << "Compilation of '" << input << "' failed!" << std::endl;
			return false;

		}

		for( auto& str : headerData ) {

			int s = str.size();

			if( s > 3 && str[ s - 1 ] == 'q' && str[ s - 2 ] == 's' && str[ s - 3 ] == '.' ) {

				(*dependencies).push_back( get_absolute_path( str, base ) );

			}else{

				(*natives).push_back( str );

			}

		}

		if( verbose ) {
			std::cout << "Compiled '" << input << "' successfully!" << std::endl;
		}

		infile.close();
		return true;

	}

	std::cout << "Compilation of '" << input << "' failed!" << std::endl;
	std::cout << "No such file found!" << std::endl;
	return false;

}

bool build_tree( std::string input, std::string output, bool verbose, seq::Compiler& compiler, seq::StringTable& strings ) {

	struct CompiledUnit {
		std::vector<std::string> dependencies;
		std::vector<seq::byte> buffer;
	};

	std::vector<std::string> done;
	std::map<size_t,CompiledUnit> units;
	std::vector<std::string> natives;

	auto build_targets = [&] ( std::map<size_t,CompiledUnit>* units, std::vector<std::string>* dependencies, std::vector<std::string>* targets, std::vector<std::string>* done, std::vector<std::string>* natives ) -> bool {

		for( auto& target : *targets ) {

			if( std::find(done->begin(), done->end(), target) != done->end() ) {
				continue;
			}

			CompiledUnit unit;

			if( !build( target, &unit.buffer, &unit.dependencies, natives, verbose, compiler ) ){
				return false;
			}

			dependencies->insert( dependencies->end(), unit.dependencies.begin(), unit.dependencies.end() );

			// put compiled unit in unit map
			(*units)[ get_path_hash( target ) ] = std::move( unit );
			done->push_back( target );

		}

		return true;

	};

	std::vector<std::string> targets = { get_absolute_path( input, get_cwd_path() ) };

	// build and resolve until there is nothing more to be done (starting with 'input' ^)
	while( !targets.empty() ) {

		std::vector<std::string> dependencies;

		if( !build_targets( &units, &dependencies, &targets, &done, &natives ) ) {

			return false;

		}

		targets = std::move( dependencies );

	}

	// when reversed most dependencies are put into correct order
	std::reverse(done.begin(), done.end());

	// sort dependencies
	for( bool sorted = true; !sorted; ) {

		std::vector<std::string> seen;
		const int s = done.size();
		seen.reserve(s);
		sorted = true;

		for( int i = 0; i < s; i ++ ) {

			const auto& file = done[i];
			const auto& dependencies = units.at( get_path_hash( file ) ).dependencies;

			for( const auto& dept : dependencies ) {

				if( std::find(seen.begin(), seen.end(), dept) == seen.end() ) {

					done.erase( done.begin() + i );
					done.push_back( file );
					sorted = false;
					break;

				}

			}

			// add at the end to prevent self-reference
			seen.push_back( file );

		}

	}

	// create output file
	std::ofstream outfile( output, std::ios::binary );
	if( outfile.good() ) {

		// write Sequensa header to file
		{
			auto header = build_header_map( natives, strings );
			std::vector<seq::byte> arr;
			seq::BufferWriter bw(arr);
			bw.putFileHeader(SEQ_API_VERSION_MAJOR, SEQ_API_VERSION_MINOR, SEQ_API_VERSION_PATCH, header);
			outfile.write((char*)arr.data(), arr.size());
		}

		// write units to output file
		for( auto& file : done ) {

			auto& buf = units.at( get_path_hash( file ) ).buffer;
			outfile.write( (char*) buf.data(), buf.size() );

		}

		return true;

	}

	return false;

}

void build( ArgParse& argp, Options opt ) {

	failed = false;
	auto vars = argp.getArgs("--build", "-b");
	seq::Compiler compiler;
	seq::StringTable table;

	if( opt.optimize ) {
		compiler.setOptimizationFlags( (seq::oflag_t) seq::Optimizations::All );
		compiler.setNameTable( &table );
	}

	if( vars.size() == 2 ) {
		singular = opt.no_multi_error;
		silent = opt.no_warn;

		compiler.setErrorHandle( [] (seq::CompilerError* err) -> bool {
			if( err->isCritical() ) {
				std::cout << "Fatal: " << err->what() << std::endl;
				failed = true;
				return true;
			}

			if( err->isError() ) {
				std::cout << "Error: " << err->what() << std::endl;
				failed = true;
				return singular;
			}

			if( err->isWarning() && !silent ) {
				std::cout << "Warning: " << err->what() << std::endl;
			}

			return false;
		} );

		if( !build_tree( vars.at(0), vars.at(1), opt.verbose, compiler, table ) ) {

			std::cout << "Build failed!" << std::endl;

		}

	}else{
		USAGE_HELP("Expected two filenames!", "build");
	}

}
