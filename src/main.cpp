
#define SEQ_IMPLEMENT
#include "api/SeqAPI.hpp"

#define ARGPARSE_IMPLEMENT
#include "lib/argparse.hpp"

#include "modes.hpp"

int main( int argc, char **argv ) {

	ArgParse argp( argc, argv );
	short mode = 0;

	mode |= argp.hasFlag("--help") || argp.hasFlag("-h") ? 1 : 0;
	mode |= argp.hasFlag("--build") || argp.hasFlag("-b") ? 2 : 0;
	mode |= argp.hasFlag("--run") || argp.hasFlag("-r") ? 4 : 0;

	Options options = {0};
	options.verbose = argp.hasFlag("-v");
	options.force_execution = argp.hasFlag("-f");
	options.print_all = argp.hasFlag("-a");

	try{

		switch( mode ) {

			case 1:
				help( argp, options );
				break;

			case 2:
				build( argp, options );
				break;

			case 4:
				run( argp, options );
				break;

			default:
				std::cout << "Invalid arguments!" << std::endl;
				std::cout << "Use '--help' for usage help." << std::endl;
				break;

		}

	}catch(...){

		std::cout << "Unknown internal error occurred, process terminated!" << std::endl;

	}

	return 0;

}
