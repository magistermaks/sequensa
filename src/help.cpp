
#include "modes.hpp"
#include "api/SeqAPI.hpp"

void help( ArgParse& argp, Options opt ) {

	std::cout << "Options:" << std::endl;
	std::cout << "  Use --help  to see this help page." << std::endl;
	std::cout << "  Use --build to compile Sequensa program." << std::endl;
	std::cout << "  Use --run   to execute compiled Sequensa program." << std::endl;
	std::cout << "  Use -v      to enable verbose mode." << std::endl;
	std::cout << "  Use -f      to force execution regardless of version mismatch." << std::endl;
	std::cout << "  Use -a      to print entire exit stream." << std::endl;
	std::cout << std::endl;
	std::cout << "Example:" << std::endl;
	std::cout << "  sequensa --build ./main.sq ./main.sqc" << std::endl;
	std::cout << "  sequensa --run ./main.sqc" << std::endl;
	std::cout << std::endl;
	std::cout << "Version:" << std::endl;
	std::cout << "  Sequensa API " << '"' << SEQ_API_NAME << "\" " << SEQ_API_VERSION_MAJOR << '.' << SEQ_API_VERSION_MINOR << '.' << SEQ_API_VERSION_PATCH << std::endl;
	std::cout << "  Standard " << SEQ_API_STANDARD << std::endl;
	std::cout << "  Version " << SQ_VER << " (" << SQ_TARGET << ')' << std::endl;

}
