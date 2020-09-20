
#include "modes.hpp"

void build( std::string input, std::string output, bool v ) {

	std::ifstream infile( input );
	if( infile.good() ) {

		std::vector<seq::byte> arr;
		{
			std::map<seq::string, seq::string> header;
			std::vector<seq::byte> buffer;

			{
				seq::string load = ""_b;
				std::vector<seq::string> headerData;

				try{

					seq::string content( (std::istreambuf_iterator<char>(infile) ), (std::istreambuf_iterator<char>() ));
					buffer = seq::Compiler::compile(content, &headerData);

				}catch( seq::CompilerError& err ){

					std::cout << "Compilation failed!" << std::endl;
					std::cout << err.what() << (v ? " (" + input + ")" : "") << std::endl;
					return;

				}

				for( auto& str : headerData ) {
					load += str;
					load += ';'_b;
				}

				header["load"_b] = load;
			}

			header["time"_b] = std::time(0);
			header["std"_b] = (seq::byte*) SEQ_API_STANDARD;
			header["api"_b] = (seq::byte*) SEQ_API_NAME;

			seq::BufferWriter bw( arr );

			bw.putFileHeader(SEQ_API_VERSION_MAJOR, SEQ_API_VERSION_MINOR, SEQ_API_VERSION_PATCH, header);
			bw.putBuffer( buffer );
		}

		std::cout << "Compiled '" << input << "' successfully!" << (v ? " (" + input + ")" : "") << std::endl;

		std::ofstream outfile( output, std::ios::binary );
		outfile.write( (char*) arr.data(), arr.size() );
		outfile.flush();
		outfile.close();

	}else{
		std::cout << "No such file '" << input << "' found!" << std::endl;
		std::cout << "Use '-help' for usage help." << std::endl;
		return;
	}

	infile.close();

}

void build( ArgParse& argp ) {

	auto vars = argp.getValues();

	if( vars.size() == 2 ) {
		build( vars.at(0), vars.at(1), argp.hasFlag( "v" ) );
	}else{
		std::cout << "Invalid arguments!" << std::endl;
		std::cout << "Use '-help' for usage help." << std::endl;
	}

}
