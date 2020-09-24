
#include "modes.hpp"
#include "api/SeqAPI.hpp"

bool build( std::string input, std::vector<seq::byte>* buffer, std::vector<std::string>* dependencies, std::vector<std::string>* natives, bool v ) {

	std::ifstream infile( input );
	if( infile.good() ) {

		std::vector<seq::string> headerData;

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

			if( s > 3 && str.at( s - 1 ) == 'q'_b && str.at( s - 2 ) == 's'_b && str.at( s - 3 ) == '.'_b ) {

				(*dependencies).push_back( seq::util::toStdString(str) );

			}else{

				(*natives).push_back( seq::util::toStdString(str) );

			}

		}

		std::cout << "Compiled '" << input << "' successfully!" << std::endl;

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
	std::vector<std::string> natives;

	auto build_targets = [v] ( std::map<size_t,CompiledUnit>* units, std::vector<std::string>* dependencies, std::vector<std::string>* targets, std::vector<std::string>* done, std::vector<std::string>* natives ) -> bool {

		for( auto& target : *targets ) {

			if( std::find(done->begin(), done->end(), target) != done->end() ) {
				continue;
			}

			CompiledUnit unit;
			size_t hash = get_path_hash( target );

			if( !build( target, &unit.buffer, &unit.dependencies, natives, v ) ){
				return false;
			}

			dependencies->insert( dependencies->end(), unit.dependencies.begin(), unit.dependencies.end() );

			// put compiled unit in unit map
			(*units)[hash] = std::move( unit );
			done->push_back( target );

		}

		return true;

	};

	std::vector<std::string> targets = {input};

	while( !targets.empty() ) {

		std::vector<std::string> dependencies;

		if( !build_targets( &units, &dependencies, &targets, &done, &natives ) ) {
			return false;
		}

		targets = dependencies;

	}

	// TODO: sort compiled units

	// TODO: save buffers in output file

	return true;

}

void build( ArgParse& argp, Options opt ) {

	auto vars = argp.getValues();

	if( vars.size() == 2 ) {
		build_tree( vars.at(0), vars.at(1), opt.verbose );
	}else{
		std::cout << "Invalid arguments!" << std::endl;
		std::cout << "Use '-help' for usage help." << std::endl;
	}

}
