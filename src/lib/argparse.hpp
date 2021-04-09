
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

#ifndef ARGPARSE_HPP_
#define ARGPARSE_HPP_

#include <vector>
#include <string>
#include <algorithm>
#include <map>

class ArgParse {

	public:
		ArgParse( int argc, char **argv );
		ArgParse( ArgParse&& argp );

		bool hasFlag( const char* name );
		bool hasFlag( const char* name, const char* name2 );
		bool isEmpty();
		const std::string& getName();
		const std::vector<std::string> getFlags();
		const std::vector<std::string> getArgs( const char* name );
		const std::vector<std::string> getArgs( const char* name, const char* name2 );

	private:
		std::vector<std::string> flags;
		std::vector<std::string> values;
		std::map<std::string,std::vector<std::string>> params;
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
				flags.push_back( arg );
				values.clear();
			}else{
				values.push_back( arg );
			}

			if( flags.size() > 0 ) {
				params[ flags[flags.size() - 1] ] = values;
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

bool ArgParse::hasFlag( const char* name, const char* name2 ) {
	return hasFlag(name) || hasFlag(name2);
}

bool ArgParse::isEmpty() {
	return this->empty;
}

const std::string& ArgParse::getName() {
	return this->name;
}

const std::vector<std::string> ArgParse::getFlags() {
	return this->flags;
}

const std::vector<std::string> ArgParse::getArgs( const char* name ) {
	return hasFlag(name) ? this->params[name] : std::vector<std::string>();
}

const std::vector<std::string> ArgParse::getArgs( const char* name, const char* name2 ) {
	return hasFlag(name) ? this->params[name] : (hasFlag(name2) ? this->params[name2] : std::vector<std::string>());
}

#undef ARGPARSE_IMPLEMENT
#endif

#endif /* ARGPARSE_HPP_ */
