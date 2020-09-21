#ifndef VSTL_HPP_INCLUDED
#define VSTL_HPP_INCLUDED

#include <vector>
#include <functional>
#include <exception>
#include <string>
#include <iostream>

#define VSTL_MODE_STRICT 2
#define VSTL_MODE_LENIENT 1

#define ASSERT( condition, message ) if( !(condition) ) FAIL( message )
#define TEST( name, ... ) long __vstl_test__##name = vstl::Test( #name, __LINE__, [&] () -> void __VA_ARGS__ ).add();
#define FAIL( what ) throw vstl::TestFail( what );
#define CHECK_ELSE( value, expected ) for( auto a = value, b = expected; a != b; )
#define CHECK( value, expected ) CHECK_ELSE( value, expected ) FAIL( "Expected: " + std::to_string( b ) + " got: " + std::to_string( a ) )
#define REGISTER_EXCEPTION( id, name ) long __vstl_exception__##id = vstl::register_exception( [&] (std::exception_ptr p) -> void { try{ if( p ) std::rethrow_exception(p); } catch( name& e ) { throw vstl::TestFail( e.what() ); } catch ( ... ) {} } )
#define BEGIN( mode ) int main() { vstl::run( mode ); }

namespace vstl {

    class TestFail: public std::exception {

        public:
            explicit TestFail ( const std::string& error );
            virtual const char* what() const throw();

        private:
            std::string error;
    };

    class Test {

        public:
            Test( std::string name, int line, std::function<void(void)> lambda );
            long add();
            bool run( int mode );

        private:
            const std::string name;
            const int line;
            const std::function<void(void)> lambda;

    };

    std::vector<Test> tests;
    std::vector<std::function<void(std::exception_ptr)>> exception_handles;
    int successful_count = 0;
    int failed_count = 0;
    void run( int mode );
    long register_exception( std::function<void(std::exception_ptr)> handle );

};

void vstl::run( int mode ) {
    for( Test& test : tests ) {
    	if( !test.run( mode ) && mode == VSTL_MODE_STRICT ) {
    		break;
    	}
    }

    std::cout << std::endl << "Executed " + std::to_string( vstl::failed_count + vstl::successful_count ) + " tests, " +
        std::to_string( vstl::failed_count ) + " failed, " +
        std::to_string( vstl::successful_count ) + " succeeded." << std::endl;
}

long vstl::register_exception( std::function<void(std::exception_ptr)> handle ) {
	exception_handles.push_back( handle );
	return exception_handles.size() - 1;
}


vstl::Test::Test( std::string _name, int _line, std::function<void(void)> _lambda ): name( _name ), line( _line ), lambda( _lambda ) {}

long vstl::Test::add() {
    vstl::tests.push_back( *this );
    return vstl::tests.size() - 1;
}

bool vstl::Test::run( int mode ) {
    try{
        this->lambda();
	} catch (vstl::TestFail &fail) {
        std::cerr << "Test '" + this->name + "' failed! Error message: " << fail.what() << std::endl;
        vstl::failed_count ++;
        return false;
    }catch( ... ) {
        std::exception_ptr p = std::current_exception();

        for( auto& handle : vstl::exception_handles ) {
        	try{
        		handle( p );
        	}catch( vstl::TestFail &fail ) {
        		std::cerr << "Test '" + this->name + "' failed! Error message: " << fail.what() << std::endl;
        		vstl::failed_count ++;
        		return false;
        	}
        }

        std::cerr << "Test '" + this->name + "' failed! Unregistered exception thrown! Error: " << (p ? p.__cxa_exception_type()->name() : "unknown") << std::endl;
        vstl::failed_count ++;
        return false;
    }

    std::cout << "Test '" + this->name + "' successful!" << std::endl;
    vstl::successful_count ++;
    return true;
}

vstl::TestFail::TestFail( const std::string& _error ): error( _error ) {}

const char* vstl::TestFail::what() const throw() {
    return this->error.c_str();
}

#endif // VSTL_HPP_INCLUDED
