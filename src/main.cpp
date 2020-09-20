
#define SEQ_IMPLEMENT
#include "SeqAPI.hpp"

#define ARGPARSE_IMPLEMENT
#include "argparse.hpp"

#include "modes.hpp"

int main( int argc, char **argv ) {

	ArgParse argp( argc, argv );
	short mode = 0;

	mode |= argp.hasFlag("help") || argp.hasFlag("h") ? 1 : 0;
	mode |= argp.hasFlag("build") || argp.hasFlag("b") ? 2 : 0;
	mode |= argp.hasFlag("run") || argp.hasFlag("r") ? 8 : 0;

	switch( mode ) {

		case 1:
			help( argp );
			break;

		case 2:
			build( argp );
			break;

		case 4:
			//link( argp );
			break;

		case 8:
			run( argp );
			break;

		default:
			std::cout << "Invalid arguments!" << std::endl;
			std::cout << "Use '-help' for usage help." << std::endl;
			break;

	}

	return 0;

}