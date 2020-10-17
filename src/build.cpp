
#include "modes.hpp"
#include "api/SeqAPI.hpp"

bool build( std::string input, std::vector<seq::byte>* buffer, std::vector<std::string>* dependencies, std::vector<seq::string>* natives, bool v ) {

	std::ifstream infile( input );
	if( infile.good() ) {

		std::vector<seq::string> headerData;
		std::string base = get_directory( input );

		try{

			seq::string content( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
			*buffer = seq::Compiler::compile(content, &headerData);

		}catch( seq::CompilerError& err ){

			std::cout << "Compilation of '" << input << "' failed!" << std::endl;
			std::cout << err.what() << std::endl;
			return false;

		}

		for( auto& str : headerData ) {

			int s = str.size();

			if( s > 3 && str[ s - 1 ] == 'q'_b && str[ s - 2 ] == 's'_b && str[ s - 3 ] == '.'_b ) {

				(*dependencies).push_back( get_absolute_path( seq::util::toStdString(str), base ) );

			}else{

				(*natives).push_back( str );

			}

		}

		if( v ) {
			std::cout << "Compiled '" << input << "' successfully!" << std::endl;
		}

		infile.close();
		return true;

	}

	std::cout << "Compilation of '" << input << "' failed!" << std::endl;
	std::cout << "No such file found!" << std::endl;
	return false;

}

bool build_tree( std::string input, std::string output, bool v ) {

	struct CompiledUnit {
		std::vector<std::string> dependencies;
		std::vector<seq::byte> buffer;
	};

	std::vector<std::string> done;
	std::map<size_t,CompiledUnit> units;
	std::vector<seq::string> natives;

	auto build_targets = [&] ( std::map<size_t,CompiledUnit>* units, std::vector<std::string>* dependencies, std::vector<std::string>* targets, std::vector<std::string>* done, std::vector<seq::string>* natives ) -> bool {

		for( auto& target : *targets ) {

			if( std::find(done->begin(), done->end(), target) != done->end() ) {
				continue;
			}

			CompiledUnit unit;

			if( !build( target, &unit.buffer, &unit.dependencies, natives, v ) ){
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

			// at the end to prevent self-reference
			seen.push_back( file );

		}

	}

	// create output file
	std::ofstream outfile( output, std::ios::binary );
	if( outfile.good() ) {

		// write Sequensa header to file
		{
			std::map<seq::string, seq::string> header;

			{
				seq::string load;

				for( auto& native : natives ) load += native + ';'_b;
				if( !load.empty() ) load.pop_back();

				header["load"_b] = load;
			}

			header["api"_b] = (seq::byte*) SEQ_API_NAME;
			header["std"_b] = (seq::byte*) SEQ_API_STANDARD;
			header["time"_b] = (seq::byte*) std::to_string( std::time(0) ).c_str();
			header["sys"_b] = (seq::byte*) SQ_TARGET;

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

	auto vars = argp.getValues();

	if( vars.size() == 2 ) {

		if( !build_tree( vars.at(0), vars.at(1), opt.verbose ) ) {

			std::cout << "Build failed!" << std::endl;

		}

	}else{

		std::cout << "Expected two filenames!" << std::endl;
		std::cout << "Use '--help' for usage help." << std::endl;

	}

}
