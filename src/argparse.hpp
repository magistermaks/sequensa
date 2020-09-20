
#ifndef ARGPARSE_HPP_
#define ARGPARSE_HPP_

#include <vector>
#include <string>
#include <algorithm>

class ArgParse {

	public:
		ArgParse( int argc, char **argv );
		ArgParse( ArgParse&& argp );

		bool hasFlag( const char* name );
		bool isEmpty();
		const std::string& getName();
		const std::vector<std::string>& getFlags();
		const std::vector<std::string>& getValues();

	private:
		std::vector<std::string> flags;
		std::vector<std::string> values;
		std::string name;
		bool empty;

};

#ifdef ARGPARSE_IMPLEMENT

ArgParse::ArgParse( int argc, char **argv ) {

	if( argc > 0 ) {

		this->name = std::string( argv[0] );

		for( int i = 1; i < argc; i ++ ) {

			std::string arg( argv[i] );

			if( arg[0] == '-' ) {
				flags.push_back( arg.substr(1) );
			}else{
				values.push_back( arg );
			}

		}

	}

	this->empty = (argc <= 1);

}

ArgParse::ArgParse( ArgParse&& argp ) {
	this->flags = std::move( argp.flags );
	this->values = std::move( argp.values );
	this->name = std::move( argp.name );
	this->empty = std::move( argp.empty );
}

bool ArgParse::hasFlag( const char* name ) {
	return std::find(this->flags.begin(), this->flags.end(), std::string(name)) != this->flags.end();
}

bool ArgParse::isEmpty() {
	return this->empty;
}

const std::string& ArgParse::getName() {
	return this->name;
}

const std::vector<std::string>& ArgParse::getFlags() {
	return this->flags;
}

const std::vector<std::string>& ArgParse::getValues() {
	return this->values;
}

#undef ARGPARSE_IMPLEMENT
#endif

#endif /* ARGPARSE_HPP_ */
