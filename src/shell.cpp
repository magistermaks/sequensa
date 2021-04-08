
#include "api/SeqAPI.hpp"
#include "modules.hpp"

seq::Stream seq_std_outln( seq::Stream& input ) {
	for( auto& element : input ) {
		std::cout << (seq::byte*) seq::util::stringCast( element ).String().getString().c_str();
	}
	return {};
}

void shell( ArgParse& argp, Options opt ) {

	seq::Executor exe;
	exe.setStrictMath( opt.strict_math );
	seq::Compiler compiler;

	// create header for loading libs
	seq::StringTable strings = {};
	seq::StringTable loads = argp.getArgs("--shell");
	seq::FileHeader header( SEQ_API_VERSION_MAJOR, SEQ_API_VERSION_MINOR, SEQ_API_VERSION_PATCH, build_header_map( loads, strings ) );

	seq::Stream args = { seq::util::newNull() };

	if( !load_native_libs( exe, header, opt.verbose ) ) {
		std::cout << "Failed to create virtual environment, start aborted!" << std::endl;
		return;
	}

	std::cout << "Sequensa " << SEQ_API_VERSION_MAJOR << "." << SEQ_API_VERSION_MINOR << "." << SEQ_API_VERSION_PATCH << " (" << SEQ_API_STANDARD << ")" << std::endl;
	std::cout << "Use CTRL+C to exit." << std::endl << std::endl;

	std::vector<std::vector<seq::byte>> buffers;

	while(true) {

		try{

			std::cout << "<< ";

			std::string str;
			std::getline(std::cin, str);

			buffers.push_back( compiler.compile(str) );
			auto& buf = buffers.at( buffers.size() - 1 );
			seq::ByteBuffer bb(buf.data(), buf.size());

			std::cout << ">> ";
			exe.execute(bb, args, false);

			if( exe.getResults().size() != 0 ) {
				std::cout << exe.getResultString();
				exe.getResults().clear();
			}

			std::cout << std::endl;

		}catch(std::exception& err) {
			std::cout << "Error: " << err.what() << std::endl;
		}

	}

	unload_native_libs();

}
