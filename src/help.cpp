
#include "modes.hpp"
#include "api/SeqAPI.hpp"

#define SQ_VER "1.0"
#define SQ_TARGET "linux"

void help( ArgParse& argp ) {

	std::cout << "Options:" << std::endl;
	std::cout << "  Use `-help`  to see this help page." << std::endl;
	std::cout << "  Use `-build` to compile Sequensa program." << std::endl;
	std::cout << "  Use `-run`   to execute compiled Sequensa program." << std::endl;
	std::cout << "  Use `-v`     to enable verbose mode." << std::endl;
	std::cout << std::endl;
	std::cout << "Example:" << std::endl;
	std::cout << "  sq -build ./main.sq" << std::endl;
	std::cout << "  sq -run ./main.sqc" << std::endl;
	std::cout << std::endl;
	std::cout << "Version:" << std::endl;
	std::cout << "  Sequensa API " << '"' << SEQ_API_NAME << "\" " << SEQ_API_VERSION_MAJOR << '.' << SEQ_API_VERSION_MINOR << '.' << SEQ_API_VERSION_PATCH << std::endl;
	std::cout << "  Standard " << SEQ_API_STANDARD << std::endl;
	std::cout << "  Version " << SQ_VER << " (" << SQ_TARGET << ')' << std::endl;

}
