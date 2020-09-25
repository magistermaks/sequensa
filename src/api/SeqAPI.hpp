
/*
 * MIT License
 *
 * Copyright (c) 2020 magistermaks
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

/*
 * Copyright Appendix:
 * The following Software is licensed under the MIT license written above,
 * but the Sequensa Language with all its specification is copyrighted by darktree.net
 * Thus you may only redistribute the Software if your copy does not violate that license.
 */

/*
 * General API overview:
 *
 * 		For more informations about Sequensa Language itself see:
 * 		http://darktree.net/projects/sequensa/
 *
 * 1. Compiling and executing
 *
 *		seq::Compiler::compile( seq::string code, std::vector<seq::string>* headerData = nullptr )
 *		The seq::Compiler::compile function can be used to generate executable Sequesa binary buffer.
 *		If provided with the `headerData` pointer it will store an array of found dependence names in it.
 * 		The returned buffer is of type `std::vector<seq::byte> `
 *
 * 		To execute returned buffer it first must encased in ByteBuffer object, the first expected constructor
 * 		argument is the pointer to the buffer, and the second one is the buffer size:
 * 		seq::ByteBuffer bb( buf.data(), buf.size() )
 * 		where `buf` is the std::vector returned by seq::Copiler::compile.
 *
 * 		Then the ByteBuffer can be passed to the seq::Executor's execute method:
 * 		Full example:
 *
 * 			// Compile the code and save bytecode in the ByteBuffer
 * 			auto buf = seq::Compiler::compile( code_to_excecute );
 * 			seq::ByteBuffer bb( buf.data(), buf.size() );
 *
 * 			// Execute the bytecode
 * 			seq::Executor exe;
 * 			exe.execute( bb );
 *
 * 2. Execution argument(s)
 *
 *		The seq::Excecutor's execute method can also accept additional argument 'args'
 *		which contains the stream of top-level arguments (using more than one argument is
 *		unadvised, as it will cause the whole Sequensa program to execute multiple times - ones per argument)
 *		The default value is an one-element stream containing the value 'null'
 *
 *			// Create args stream containing the number '2'
 *			seq::Stream args = {
 *				seq::Generic( new seq::type::Number( false, 2 ) )
 * 			};
 *
 * 			// Execute the bytecode with given args
 * 			seq::Executor exe;
 * 			exe.execute( bb, args );
 *
 * 3. Injecting (and removing) functions to/from Sequensa
 *
 * 		By default nothing outside of the Sequnsa program can be manipulated by SVM
 * 		(Sequensa Virtual Machine) to allow function to be invoked from inside of the
 * 		Sequensa program it must first be injected into the executor using the 'inject' method.
 *
 * 		`inject` method takes two parameters: name of the function inside of the Sequensa and the
 * 		function pointer (seq::type::Native). the given function must take seq::Stream as an argument and return
 * 		seq::Stream.
 *
 * 			exe.inject( "myfunc"_b, [] (seq::Stream args) -> seq::Stream {
 * 				// ...
 * 			} );
 *
 * 		All injected function can be removed from the Executor object by
 * 		calling `executor.reset()`
 *
 * 4. Obtaining results
 *
 * 		After successful execution (exe.execute) of the program the returned value(s) can be
 * 		obtained by calling `exe.getResult()` for single result or `exe.getResults()` for stream
 * 		of returned values. Additional helper method `exe.getResultString()` can be used to obtain result
 * 		directly as c++ string.
 *
 * 		Note: `exe.getResult()` returns first element of the result stream (according to
 * 		Sequensa Language Specification this is the program's exit code)
 *
 * 5. Using Sequensa streams and data types
 *
 * 		seq::Stream is a std::vector of seq::Generic's used to represent Sequensa streams (for both input and output)
 * 		seq::Generic is an object used to represent Sequensa's data types and provide a simple interface to manipulate them.
 *
 * 			seq::Generic val = exe.getResult();
 *
 * 			// use this to get the Generic's type
 * 			seq::DataType type = val.getDataType();
 *
 * 			// If you know the type of the value you can access it
 * 			// Never directly access generics without checking the type - that would cause an
 * 			// undefined behavior and most probably crash your program.
 * 			if( type == seq::DataType::Number ) {
 *
 * 				std::cout << val.Number().getLong();
 *
 * 			}
 *
 * 6. Exceptions and their meaning
 *
 * 		Sequnesa API can generate 3 types of exceptions:
 *
 * 		seq::CompilerError
 * 			Error generated only by the seq::Compile set of functions,
 * 			it signals the the compilation was unsuccessful and provides a standard error message.
 *
 * 		seq::RuntimeError
 * 			Generated by executor.execute method (and a set of private method located in the Executor class),
 * 			it signals that the program execution was unsuccessful, return value of `exe.getResults()` and
 * 			`exe.getResult()` are undefined after this exception.
 *
 * 		seq::InternalError
 * 			Can be caused by many Sequensa API methods it can signal one of the following problems:
 * 				1. Invalid Sequensa bytecode (file corruption etc)
 * 				2. Programmer error - either on the side of the API itself or the user.
 * 				3. Some other completely unexpected thing.
 * 			It is advised to report those exceptions as bugs.
 *
 * 7. seq::string, seq::byte and c++11 literals
 *
 * 		seq::byte is alias for unsigned char,
 * 		seq::string is a std::basic_string of seq::bytes it can be used to store utf-8 data
 * 		(on linux and macos char is unsigned by default making it 100% to safe to cast between
 * 		std::string and seq::string using the seq::util::toStdString and seq::util::toSeqString functions,
 * 		but it should still work correctly on windows at least as long as only ASCII data is used)
 *
 * 		To create seq::string the following syntax can be used:
 * 			seq::string text = "Hello Text!"_b;
 *
 * 		The `_b` literal can also be used to create seq::bytes:
 * 			seq::byte b = 'A'_b;
 *
 * 8. Sequensa API meta-data
 *
 * 		All meta-data can be found in the SEQ_API_* macros
 *
 * 		SEQ_API_STANDARD - string - version of the Sequensa Standard implemented by the API
 * 		SEQ_API_NAME - string - name of the API
 * 		SEQ_API_VERSION_MAJOR - byte - Major component of the version number
 * 		SEQ_API_VERSION_MINOR - byte - Minor component of the version number
 * 		SEQ_API_VERSION_PATCH - byte - Patch component of the version number
 */

/*
 * Preprocessor:
 * 	#define SEQ_IMPLEMENT 			to implement the Sequensa API
 * 	#define SEQ_EXCLUDE_COMPILER 	to exclude compiler code from API
 */

#pragma once

#include <inttypes.h>
#include <exception>
#include <string>
#include <math.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <functional>
#include <algorithm>
#include <regex>

#define SEQ_MIN_OPCODE 1
#define SEQ_MAX_OPCODE 15
#define SEQ_MIN_DATA_TYPE 1
#define SEQ_MAX_DATA_TYPE 12
#define SEQ_MIN_CALL_TYPE 1
#define SEQ_MAX_CALL_TYPE 6
#define SEQ_MIN_OPERATOR 1
#define SEQ_MAX_OPERATOR 20
#define SEQ_MAX_UBYTE1 255l
#define SEQ_MAX_UBYTE2 65535l
#define SEQ_MAX_UBYTE4 4294967295l
#define SEQ_MAX_UBYTE8 18446744073709551615l
#define SEQ_TAG_FIRST 1
#define SEQ_TAG_LAST 2
#define SEQ_TAG_END 4

#define SEQ_API_STANDARD "2020-07-15"
#define SEQ_API_VERSION_MAJOR 1
#define SEQ_API_VERSION_MINOR 0
#define SEQ_API_VERSION_PATCH 6
#define SEQ_API_NAME "SeqAPI"

namespace seq {

    /// define "byte" (unsigned char)
    typedef uint8_t byte;

    /// define own version of string to fix windows implementation errors
    typedef std::basic_string<byte> string;

}

constexpr seq::byte operator"" _b ( char chr ) {
    return (seq::byte) chr;
}

constexpr seq::byte* operator"" _b ( const char* str, std::size_t siz ) {
    return (seq::byte*) str;
}

/// define std::hash for seq::string to be used by std::unordered_map
namespace std {

    template<> struct hash<seq::string> {

        std::size_t operator()(seq::string const& s) const noexcept {

            // This implementation can cause problems for UTF-8 strings on Windows
            // but it is used only for variable names, with allow only ASCII chars
            return std::hash<std::string>{}( (char*) s.c_str() );
        }

    };

}

namespace seq {

    /// forward definition of all sequensa API classes
    class FileHeader;
    class ByteBuffer;
    class BufferReader;
    class TokenReader;
    class InternalError;
    class CompilerError;
    class RuntimeError;
    class Executor;
    class FlowCondition;

    /// Opcodes - operation identifiers
    enum struct Opcode: byte {
        BLT = 1,  // BLT ;
        BLF = 2,  // BLF ;
        NIL = 3,  // NIL ;
        NUM = 4,  // NUM [HEAD] [TAIL...] ;
        INT = 5,  // INT [BYTE] ;
        STR = 6,  // STR [UTF8...] [NULL] ;
        TYP = 7,  // TYP [TYPE] ;
        VMC = 8,  // VMC [TYPE] ;
        ARG = 9,  // ARG [SIZE] ;
        FUN = 10, // FUN [HEAD] [TAIL...] [BODY...] ;
        EXP = 11, // EXP [TYPE] [HEAD] [TAIL...] [L...] [R...] ;
        VAR = 12, // VAR [ASCI...] [NULL] ;
        DEF = 13, // DEF [ASCI...] [NULL] ;
        FLC = 14, // FLC [SIZE] [[SIZE] [BODY...]...] ;
        SSL = 15  // SSL [TAGS] [HEAD] [TAIL...] [BODY...] ;
    };

    /// Sequensa data types
    enum struct DataType: byte {
        Bool   = 1, // maps to: BLT BLF
        Null   = 2, // maps to: NIL
        Number = 3, // maps to: NUM INT
        String = 4, // maps to: STR
        Type   = 5, // maps to: TYP
        VMCall = 6, // maps to: VMC
        Arg    = 7, // maps to: ARG
        Func   = 8, // maps to: FUN
        Expr   = 9, // maps to: EXP
        Name  = 10, // maps to: VAR DEF
        Flowc = 11, // maps to: FLC
        Stream = 12 // maps to: SSL
    };

    enum struct ExprOperator: byte {
    	Less           = 1, // <
        Greater        = 2, // >
        Equal          = 3, // =
        NotEqual       = 4, // !=
        NotGreater     = 5, // !>, <=
        NotLess        = 6, // !<, >=
        And            = 7, // &&
        Or             = 8, // ||
        Xor            = 9, // ^^
        Not            = 10, // !
        Multiplication = 11, // *
        Division       = 12, // /
        Addition       = 13, // +
        Subtraction    = 14, // -
        Modulo         = 15, // %
        Power          = 16, // **
        BinaryAnd      = 17, // &
        BinaryOr       = 18, // |
        BinaryXor      = 19, // ^
        BinaryNot      = 20  // ~
    };

    /// Simple fraction struct, used by seq::type::Number
    struct Fraction {
        const long numerator;
        const long denominator;
    };

    namespace type {

        class Null;
        class Stream;
        class Flowc;
        class Expression;
        class Function;
        class Name;
        class VMCall;
        class Type;
        class String;
        class Number;
        class Arg;
        class Bool;
        class Generic;

        class Generic {

        	private:
                const DataType type;

        	protected:
        		Generic( const DataType type, bool anchor );
        		bool anchor;

            public:
                virtual ~Generic() {}
                const DataType getDataType();
                bool getAnchor();
                void setAnchor( bool anchor );
        };

        class Bool: public Generic {

            public:
                Bool( bool anchor, bool value );
                const bool getBool();

            private:
                const bool value;
        };

        class Number: public Generic {

            public:
                Number( bool anchor, long numerator, long denominator );
                Number( bool anchor, double value );
                const double getDouble();
                const long getLong();
                const bool isNatural();
                const Fraction getFraction();
                static byte sizeOf( unsigned long value );

            private:
                const double value;
        };

        class Arg: public Generic {

            public:
                Arg( bool anchor, byte level );
                const byte getLevel();

            private:
                const byte level;
        };

        class String: public Generic {

            public:
                String( bool anchor, const byte* value );
                string& getString();

            private:
                string value;
        };

        class Type: public Generic {

            public:
                Type( bool anchor, DataType value );
                const DataType getType();

            public:
                const DataType value;
        };

        class VMCall: public Generic {

            public:
                enum struct CallType: byte {
                    Return = 1,
                    Break = 2,
                    Exit = 3,
                    Again = 4,
                    Emit = 5,
					Final = 6
                };

                VMCall( bool anchor, CallType value );
                const CallType getCall();

            private:
                const CallType value;

        };

        class Name: public Generic {

            public:
                Name( bool anchor, bool define, string name );
                string& getName();
                const bool getDefine();

            private:
                const bool define;
                string name;
        };

        class Function: public Generic {

            public:
                Function( bool anchor, BufferReader* reader );
                Function( const Function& func );
                ~Function();
                BufferReader& getReader();

            private:
                BufferReader* reader;
        };

        class Expression: public Generic {

            public:
                Expression( bool anchor, ExprOperator op, BufferReader* left, BufferReader* right );
                Expression( const Expression& expr );
                ~Expression();
                const ExprOperator getOperator();
                BufferReader& getLeftReader();
                BufferReader& getRightReader();

            private:
                const ExprOperator op;
                BufferReader* left;
                BufferReader* right;
        };

        class Flowc: public Generic {

            public:
                Flowc( bool anchor, const std::vector<FlowCondition*> readers );
                Flowc( const Flowc& flowc );
                ~Flowc();
                const std::vector<FlowCondition*>& getConditions();

            private:
                const std::vector<FlowCondition*> conditions;
        };

        class Stream: public Generic {

            public:
                Stream( bool anchor, byte tags, BufferReader* reader );
                Stream( const Stream& stream );
                ~Stream();
                const bool machesTags( byte tags );
                BufferReader& getReader();

            private:
                const byte tags;
                BufferReader* reader;
        };

        class Null: public Generic {

            public:
                Null( bool anchor );

        };

    }

    class Generic {

    	public:
    		Generic();
    		Generic( type::Generic* generic );
    		Generic( const seq::Generic& generic );
    		Generic( seq::Generic&& generic );
    		~Generic();

    		Generic& operator= ( const Generic& generic );
    		Generic& operator= ( Generic&& generic ) noexcept;

    		const DataType getDataType();
    		bool getAnchor();
    		void setAnchor( bool anchor );

    		type::Null& Null();
			type::Stream& Stream();
    		type::Flowc& Flowc();
    		type::Expression& Expression();
    		type::Function& Function();
    		type::Name& Name();
    		type::VMCall& VMCall();
    		type::Type& Type();
    		type::String& String();
    		type::Number& Number();
    		type::Arg& Arg();
    		type::Bool& Bool();

    		type::Generic* getRaw();

    	private:
    		type::Generic* generic;

    };

    namespace type {

    	/// define Sequensa native function signature
        typedef std::vector<seq::Generic>(*Native)(std::vector<seq::Generic>);

    }

    namespace util {

        byte packTags( long pos, long end );
        std::string toStdString( seq::string str );
        seq::string toSeqString( std::string str );
        long whole( double val );
        type::Generic* copyGeneric( type::Generic* entity );
        seq::Generic numberCast( seq::Generic arg );
        seq::Generic boolCast( seq::Generic arg );
        seq::Generic stringCast( seq::Generic arg );
        std::vector<FlowCondition*> copyFlowConditions( std::vector<FlowCondition*> conditions );
        seq::Fraction asFraction( double value );
        seq::DataType toDataType( seq::string str );

    }

    /// define shorthand for stream
    typedef std::vector<seq::Generic> Stream;

    /// Internal API error
    class InternalError: public std::exception {

        private:
            std::string error;

        public:
            explicit InternalError ( const std::string& error );
            virtual const char* what() const throw();
    };

    /// Runtime error
    class RuntimeError: public std::exception {

        private:
            std::string error;

        public:
            explicit RuntimeError ( const std::string& error );
            virtual const char* what() const throw();
    };

    /// Executor Interruption
    class ExecutorInterrupt: public std::exception {

        private:
            byte code;

        public:
            explicit ExecutorInterrupt ( const byte code );
            virtual const char* what() const throw();
            byte getCode();
    };

    /// Compilation error
    class CompilerError: public std::exception {

        private:
            std::string error;

        public:
            explicit CompilerError ( const std::string& unexpected, const std::string& expected, const std::string& structure, int line );
            virtual const char* what() const throw();
    };

    class FileHeader {

        public:
            FileHeader( byte seq_major, byte seq_minor, byte seq_patch, std::map<seq::string, seq::string> properties );
            const bool checkVersion( byte seq_major, byte seq_minor );
            const bool checkPatch( byte seq_patch );
            seq::string& getValue( const byte* key );
            int getVersionMajor();
            int getVersionMinor();
            int getVersionPatch();

        private:
            const byte seq_major;
            const byte seq_minor;
            const byte seq_patch;
            std::map<seq::string, seq::string> properties;
    };

    /// token reader
    class TokenReader {

        public:
            TokenReader( BufferReader& reader );
            DataType getDataType();
            bool isAnchored();
            seq::Generic& getGeneric();

        private:
            BufferReader& reader;
            byte header;
            bool anchor;
            DataType type;
            seq::Generic generic;

            DataType getDataType( byte header );
            type::Bool* loadBool();
            type::Number* loadNumber();
            type::Arg* loadArg();
            type::String* loadString();
            type::Type* loadType();
            type::VMCall* loadCall();
            type::Name* loadName();
            type::Function* loadFunc();
            type::Expression* loadExpr();
            type::Flowc* loadFlowc();
            type::Stream* loadStream();
    };

    /// Byte buffer
    class ByteBuffer {

        public:
            ByteBuffer( byte* buffer, long length );
            BufferReader getReader();
            BufferReader getReader( long first, long last );

        private:
            byte* pointer;
            long length;
    };

    class BufferReader {

        public:
            BufferReader( byte* buffer, long first, long last );
            byte nextByte();
            byte peekByte();
            bool hasNext();
            TokenReader next();
            BufferReader* nextBlock( long length );
            long nextInt();
            FileHeader getHeader();
            Stream readAll();
            ByteBuffer getSubBuffer();

        private:
            long first;
            long last;
            long position;
            byte* pointer;
    };

    class BufferWriter {

        public:
            BufferWriter( std::vector<byte>& buffer );
            void putByte( byte b );
            void putNull( bool anchor );
            void putBool( bool anchor, bool value );
            void putNumber( bool anchor, Fraction f );
            void putArg( bool anchor, byte level );
            void putString( bool anchor, const byte* str );
            void putType( bool anchor, DataType type );
            void putCall( bool anchor, type::VMCall::CallType type );
            void putName( bool anchor, bool define, const byte* name );
            void putFunc( bool anchor, std::vector<byte>& buffer );
            void putExpr( bool anchor, ExprOperator op, std::vector<byte>& left, std::vector<byte>& right );
            void putFlowc( bool anchor, std::vector<std::vector<byte>>& buffers );
            void putStream( bool anchor, byte tags, std::vector<byte>& buf );
            void putFileHeader( byte seq_major, byte seq_minor, byte seq_patch, const std::map<seq::string, seq::string>& data );

        public:
            // usage of this function is unadvised but not forbidden
            void putBuffer( std::vector<byte>& buffer );

        private:
            void putString( const byte* str );
            void putOpcode( bool anchor, seq::Opcode code );
            void putInteger( byte length, long value );
            void putHead( byte left, byte right );
            std::vector<byte>& buffer;
    };

    class StackLevel {

        public:
            StackLevel( seq::Generic arg );
            StackLevel( StackLevel&& level );
            seq::Generic getArg();
            Stream getVar( seq::string& name, bool anchor );
            void setVar( seq::string& name, Stream value );
            void setArg( seq::Generic arg );

        private:
            seq::Generic arg;
            std::unordered_map<seq::string, Stream> vars;
    };

    class FlowCondition {
        public:
            enum struct Type: byte {
                Type = 1,
                Value = 2,
                Range = 3
            };

            FlowCondition( Type type, seq::Generic a, seq::Generic b );
            bool validate( seq::Generic arg );

            Type type;
            seq::Generic a;
            seq::Generic b;
    };

    class CommandResult {
        public:
            enum struct ResultType: byte {
                Return = 1,
                Break = 2,
                Exit = 3,
                Again = 4,
                None = 5,
				Final = 6
            };

            CommandResult( ResultType stt, Stream acc );

            ResultType stt;
            Stream acc;
    };

    class Executor {

        public:
            void inject( seq::string name, seq::type::Native native );
            StackLevel* getLevel( int level );
            StackLevel* getTopLevel();
            void reset();
            std::string getResultString();
            seq::Generic getResult();
            seq::Stream& getResults();
            void execute( ByteBuffer bb, seq::Stream args = { seq::Generic( new type::Null( false ) ) } );

        private: // use these methods only if you know what you are doing //
            void exit( seq::Stream& stream, byte code );
            Stream executeFunction( BufferReader br, Stream& stream );
            CommandResult executeCommand( TokenReader* br, byte tags );
            CommandResult executeStream( Stream& stream );
            Stream executeAnchor( Generic entity, Stream& input_stream );
            Generic executeExprPair( Generic left, Generic right, ExprOperator op, bool anchor );
            Generic executeExpr( Generic entity );
            Stream resolveName( string& name, bool anchor );
            Stream executeFlowc( std::vector<FlowCondition*> fcs, Stream& input_stream );
            Generic executeCast( Generic cast, Generic arg );

        private:
            std::unordered_map<string, type::Native> natives;
            std::vector<StackLevel> stack;
            seq::Stream result;
    };

#ifndef SEQ_EXCLUDE_COMPILER
    namespace Compiler {

    	class Token {

    		public:
    			enum class Category: byte {
    				Tag = 0,
    				Set = 1,
					Load = 2,
					Bool = 3,
					Null = 4,
					Type = 5,
					Name = 6,
					Number = 7,
					Stream = 8,
					Operator = 9,
					String = 10,
					FuncBracket = 11,
					FlowBracket = 12,
					MathBracket = 13,
					Comma = 14,
					Colon = 15,
					Arg = 16,
					VMC = 17
    			};

    			Token( unsigned int line, long data, bool anchor, Category category, seq::string& raw, seq::string& clean );
    			Token( const Token& token );
    			Token( Token&& token );
    			const unsigned int getLine();
    			const Category getCategory();
    			const seq::string& getRaw();
    			const seq::string& getClean();
    			const bool isPrimitive();
    			const long getData();
    			const bool getAnchor();
    			std::string toString();

    		private:
    			const unsigned int line;
    			const long data;
    			const bool anchor;
    			const Category category;
    			const seq::string raw;
    			const seq::string clean;

    	};

    	std::vector<byte> compile( seq::string code, std::vector<seq::string>* headerData = nullptr );

    	std::vector<Token> tokenize( seq::string code );
    	Token construct( seq::string raw, unsigned int line );
    	int findStreamEnd( std::vector<Token>& tokens, int start, int end );
    	int findOpening( std::vector<Token>& tokens, int index, Token::Category type );
    	int findClosing( std::vector<Token>& tokens, int index, Token::Category type );

    	std::vector<byte> assembleStream( std::vector<Token>& tokens, int start, int end, byte tags );
    	std::vector<byte> assemblePrimitive( Token );
    	std::vector<byte> assembleFlowc( std::vector<Token>& tokens, int start, int end, bool anchor );
    	std::vector<byte> assembleExpression( std::vector<Token>& tokens, int start, int end, bool anchor );
    	std::vector<byte> assembleFunction( std::vector<Token>& tokens, int start, int end, bool anchor );
    	int extractHeaderData( std::vector<Token>& tokens, std::vector<seq::string>* arrayPtr );

    };
#endif // SEQ_EXCLUDE_COMPILER

}

using seq::byte;

#ifdef SEQ_IMPLEMENT

byte seq::util::packTags( long pos, long end ) {
    byte tags = 0;

    tags |= ( ( pos == 0 ) ? SEQ_TAG_FIRST : 0 );
    tags |= ( ( pos == end - 1 ) ? SEQ_TAG_LAST : 0 );
    tags |= ( ( pos == end ) ? SEQ_TAG_END : 0 );

    return tags;
}

std::string seq::util::toStdString( seq::string str ) {
    return std::string( (char*) str.c_str() );
}

seq::string seq::util::toSeqString( std::string str ) {
    return seq::string( (byte*) str.c_str() );
}

long seq::util::whole( double val ) {
    return (long) val;
}

seq::type::Generic* seq::util::copyGeneric( seq::type::Generic* entity ) {
	switch( entity->getDataType() ) {
		case seq::DataType::Arg: return new seq::type::Arg( *(seq::type::Arg*) entity );
		case seq::DataType::Bool: return new seq::type::Bool( *(seq::type::Bool*) entity );
		case seq::DataType::Expr: return new seq::type::Expression( *(seq::type::Expression*) entity );
		case seq::DataType::Flowc: return new seq::type::Flowc( *(seq::type::Flowc*) entity );
		case seq::DataType::Func: return new seq::type::Function( *(seq::type::Function*) entity );
		case seq::DataType::Name: return new seq::type::Name( *(seq::type::Name*) entity );
		case seq::DataType::Null: return new seq::type::Null( *(seq::type::Null*) entity );
		case seq::DataType::Number: return new seq::type::Number( *(seq::type::Number*) entity );
		case seq::DataType::Stream: return new seq::type::Stream( *(seq::type::Stream*) entity );
		case seq::DataType::String: return new seq::type::String( *(seq::type::String*) entity );
		case seq::DataType::Type: return new seq::type::Type( *(seq::type::Type*) entity );
		case seq::DataType::VMCall: return new seq::type::VMCall( *(seq::type::VMCall*) entity );
	}

	throw seq::InternalError( "Impossible state!" );
}

seq::Generic seq::util::numberCast( seq::Generic arg ) {
	seq::type::Number* num = nullptr;

	switch( arg.getDataType() ) {

		case seq::DataType::Number: return arg;

		case seq::DataType::Bool:
	    	if( arg.Bool().getBool() ) {
	        	num = new seq::type::Number( false, 1 );
	    	}else{
	        	num = new seq::type::Number( false, 0 );
	    	}
	        break;

	    case seq::DataType::Null:
	    	num = new seq::type::Number( false, 0 );
	    	break;

	    case seq::DataType::String:
	    	try {
	    		num = new seq::type::Number( false, std::stod( seq::util::toStdString( arg.String().getString() ) ) );
	    	} catch (std::invalid_argument &err) {
	    		num = new seq::type::Number( false, 0 );
	    	}
	    	break;

	    case seq::DataType::VMCall:
	    case seq::DataType::Flowc:
	    case seq::DataType::Func:
	    	num = new seq::type::Number( false, 1 );
	        break;

	    // invalid casts: (stream, name, expr, arg)
	    default: throw seq::InternalError( "Invalid cast!" );

	}

	return seq::Generic( num );
}

seq::Generic seq::util::boolCast( seq::Generic arg ) {
	if( arg.getDataType() == seq::DataType::Bool ) return arg;

	bool val = ( seq::util::numberCast( arg ).Number().getDouble() != 0 );

	return seq::Generic( new seq::type::Bool( false, val ) );
}

seq::Generic seq::util::stringCast( seq::Generic arg ) {
	seq::type::String* str = nullptr;

	switch( arg.getDataType() ) {

		case seq::DataType::String: return arg;

		case seq::DataType::Bool:
			if( arg.Bool().getBool() ) {
				str = new seq::type::String( false, "true"_b );
			}else{
				str = new seq::type::String( false, "false"_b );
			}
			break;

		case seq::DataType::Null:
			str = new seq::type::String( false, "null"_b );
			break;

		case seq::DataType::Number:
			str = new seq::type::String( false, seq::util::toSeqString(
					arg.Number().isNatural() ?
							std::to_string( arg.Number().getLong() ) :
							std::to_string( arg.Number().getDouble() ) ).c_str()  );
			break;

		case seq::DataType::Flowc:
			str = new seq::type::String( false, "flowc"_b );
			break;

		case seq::DataType::VMCall:
		case seq::DataType::Func:
			str = new seq::type::String( false, "func"_b );
			break;

		// invalid casts: (stream, name, expr, arg)
		default: throw seq::InternalError( "Invalid cast!" );

	}

	return seq::Generic( str );
}

std::vector<seq::FlowCondition*> seq::util::copyFlowConditions( std::vector<seq::FlowCondition*> conditions ) {
	std::vector<seq::FlowCondition*> list;
	for( auto condition : conditions ) {
		list.push_back( new seq::FlowCondition( *condition ) );
	}

	return list;
}

seq::Fraction seq::util::asFraction( double value ) {
	seq::type::Number num( false, value );
	return num.getFraction();
}

seq::DataType seq::util::toDataType( seq::string str ) {
	if( str == "number"_b ) return seq::DataType::Number;
	if( str == "bool"_b ) return seq::DataType::Bool;
	if( str == "string"_b ) return seq::DataType::String;
	if( str == "type"_b ) return seq::DataType::Type;
	throw seq::InternalError( "Invalid argument!" );
}

seq::BufferWriter::BufferWriter( std::vector<byte>& _buffer ): buffer( _buffer ) {}

void seq::BufferWriter::putByte( byte b ) {
    this->buffer.push_back( b );
}

void seq::BufferWriter::putOpcode( bool anchor, seq::Opcode code ) {
    this->putByte( (byte) code | (anchor ? 0b10000000 : 0) );
}

void seq::BufferWriter::putString( const byte* str ) {
    for ( long i = 0; str[i]; i ++ ) this->putByte( str[i] );
    this->putByte( 0 );
}

void seq::BufferWriter::putInteger( byte length, long value ) {
    for( ; length != 0; length -- ) {
        this->putByte( (byte) ( value & SEQ_MAX_UBYTE1 ) );
        value = ( value >> 8 );
    }
}

void seq::BufferWriter::putHead( byte left, byte right ) {
    this->putByte( (left << 4) | right );
}

void seq::BufferWriter::putBuffer( std::vector<byte>& buf ) {
    this->buffer.insert(this->buffer.end(), buf.begin(), buf.end());
}

void seq::BufferWriter::putNull( bool anchor ) {
    this->putOpcode( anchor, seq::Opcode::NIL );
}

void seq::BufferWriter::putBool( bool anchor, bool value ) {
    this->putOpcode( anchor, (value ? seq::Opcode::BLT : seq::Opcode::BLF) );
}

void seq::BufferWriter::putNumber( bool anchor, seq::Fraction f ) {
    if( f.denominator == 1 && f.numerator >= 0 && f.numerator <= 0b01111111 ) {
        this->putOpcode( anchor, seq::Opcode::INT );
        this->putByte( (byte) f.numerator );
    }else{
    	bool sign = std::signbit( f.numerator );
    	unsigned long n = std::abs( f.numerator );

        this->putOpcode( anchor, seq::Opcode::NUM );
        byte a = seq::type::Number::sizeOf( n << 1 );
        byte b = seq::type::Number::sizeOf( f.denominator );
        this->putHead( a, b );
        this->putInteger( a, sign ? n | (1 << (a * 8 - 1)) : n );
        this->putInteger( b, f.denominator );
    }
}

void seq::BufferWriter::putArg( bool anchor, byte level ) {
    this->putOpcode( anchor, seq::Opcode::ARG );
    this->putByte( level );
}

void seq::BufferWriter::putString( bool anchor, const byte* str ) {
    this->putOpcode( anchor, seq::Opcode::STR );
    this->putString( str );
}

void seq::BufferWriter::putType( bool anchor, DataType type ) {
    this->putOpcode( anchor, seq::Opcode::TYP );
    this->putByte( (byte) type );
}

void seq::BufferWriter::putCall( bool anchor, type::VMCall::CallType type ) {
    this->putOpcode( anchor, seq::Opcode::VMC );
    this->putByte( (byte) type );
}

void seq::BufferWriter::putName( bool anchor, bool define, const byte* name ) {
    this->putOpcode( anchor, (define ? seq::Opcode::DEF : seq::Opcode::VAR) );
    this->putString( name );
}

void seq::BufferWriter::putFunc( bool anchor, std::vector<byte>& buf ) {
    this->putOpcode( anchor, seq::Opcode::FUN );
    long buffer_size = buf.size();
    byte h = seq::type::Number::sizeOf( buffer_size );
    this->putHead( h, 0 );
    this->putInteger( h, buffer_size );
    this->putBuffer( buf );
}

void seq::BufferWriter::putExpr( bool anchor, seq::ExprOperator op, std::vector<byte>& left, std::vector<byte>& right ) {
    this->putOpcode( anchor, seq::Opcode::EXP );
    this->putByte( (byte) op );
    long left_size = left.size();
    long right_size = right.size();
    byte a = seq::type::Number::sizeOf( left_size );
    byte b = seq::type::Number::sizeOf( right_size );
    this->putHead( a, b );
    this->putInteger( a, left_size );
    this->putInteger( b, right_size );
    this->putBuffer( left );
    this->putBuffer( right );
}

void seq::BufferWriter::putFlowc( bool anchor, std::vector<std::vector<byte>>& buffers ) {
    this->putOpcode( anchor, seq::Opcode::FLC );
    this->putByte( (byte) buffers.size() );
    for( auto& buf : buffers ) {
        long buffer_size = buf.size();
        byte h = seq::type::Number::sizeOf( buffer_size );
        this->putHead( h, 0 );
        this->putInteger( h, buffer_size );
        this->putBuffer( buf );
    }
}

void seq::BufferWriter::putStream( bool anchor, byte tags, std::vector<byte>& buf ) {
    this->putOpcode( anchor, seq::Opcode::SSL );
    this->putByte( tags );
    long buffer_size = buf.size();
    byte h = seq::type::Number::sizeOf( buffer_size );
    this->putHead( h, 0 );
    this->putInteger( h, buffer_size );
    this->putBuffer( buf );
}

void seq::BufferWriter::putFileHeader( byte seq_major, byte seq_minor, byte seq_patch, const std::map<seq::string, seq::string>& data ) {
    this->putByte( 's' );
    this->putByte( 'q' );
    this->putByte( 'c' );
    this->putByte(  0  );
    this->putByte( seq_major );
    this->putByte( seq_minor );
    this->putByte( seq_patch );

    seq::string data_string;
    for( const auto& x : data ) {
        data_string.append( x.first );
        data_string.push_back( 0 );
        data_string.append( x.second );
        data_string.push_back( 0 );
    }

    this->putInteger( 4, data_string.size() );

    for( const byte& b : data_string ) {
        this->putByte( b );
    }
}

seq::FileHeader::FileHeader( byte _seq_major, byte _seq_minor, byte _seq_patch, std::map<seq::string, seq::string> _properties ): seq_major( _seq_major ), seq_minor( _seq_minor ), seq_patch( _seq_patch ), properties( _properties ) {}

const bool seq::FileHeader::checkVersion( byte _seq_major, byte _seq_minor ) {
    return ( this->seq_major == _seq_major ) && ( this->seq_minor == _seq_minor );
}

const bool seq::FileHeader::checkPatch( byte _seq_patch ) {
    return ( this->seq_patch == _seq_patch );
}

seq::string& seq::FileHeader::getValue( const byte* key ) {
    return this->properties.at( seq::string(key) );
}

int seq::FileHeader::getVersionMajor() {
	return (int) this->seq_major;
}

int seq::FileHeader::getVersionMinor() {
	return (int) this->seq_minor;
}

int seq::FileHeader::getVersionPatch() {
	return (int) this->seq_patch;
}

seq::type::Generic::Generic( const DataType _type, bool _anchor ): type( _type ), anchor( _anchor ) {}

bool seq::type::Generic::getAnchor() {
    return this->anchor;
}

void seq::type::Generic::setAnchor( bool anchor ) {
    this->anchor = anchor;
}

const seq::DataType seq::type::Generic::getDataType() {
    return this->type;
}

seq::type::Bool::Bool( bool _anchor, bool _value ): seq::type::Generic( seq::DataType::Bool, _anchor ), value( _value ) {}

const bool seq::type::Bool::getBool() {
    return this->value;
}

seq::type::Arg::Arg( bool _anchor, byte _level ): seq::type::Generic( seq::DataType::Arg, _anchor ), level( _level ) {}

const byte seq::type::Arg::getLevel() {
    return this->level;
}

seq::type::Number::Number( bool _anchor, double _value ): seq::type::Generic( seq::DataType::Number, _anchor ), value( _value ) {}

seq::type::Number::Number( bool _anchor, long numerator, long denominator ): seq::type::Generic( seq::DataType::Number, _anchor ), value( (double) numerator / denominator ) {};

const double seq::type::Number::getDouble() {
    return this->value;
}

const long seq::type::Number::getLong() {
    return trunc( this->value );
}

const bool seq::type::Number::isNatural() {
	return this->getLong() == this->value;
}

const seq::Fraction seq::type::Number::getFraction() {

    long sign = ( this->value < 0 ) ? -1 : 1;
    double number = std::abs( this->value );
    double whole = std::trunc( number );
    double decimal = ( number - whole );
    long multiplier = 1;

    if( decimal > 0 )
        for( double i = decimal; i > floor( i ); i = multiplier * decimal )
            multiplier *= 10;

    long part = round( decimal * multiplier );
    long hcf = 0;
    long u = part;
    long v = multiplier;

    while( true ) {
		if( !(u %= v) ) { hcf = v; break; }
		if( !(v %= u) ) { hcf = u; break; }
	}

    multiplier /= hcf;

    return (seq::Fraction) { (long) (sign * ( (part / hcf) + (whole * multiplier) )), multiplier };
}

byte seq::type::Number::sizeOf( unsigned long value ) {
    if( value > SEQ_MAX_UBYTE4 ) return 8;
    if( value > SEQ_MAX_UBYTE2 ) return 4;
    if( value > SEQ_MAX_UBYTE1 ) return 2;
    return 1;
}

seq::type::String::String( bool _anchor, const byte* _value ): seq::type::Generic( seq::DataType::String, _anchor ), value( _value ) {}

seq::string& seq::type::String::getString() {
    return this->value;
}

seq::type::Type::Type( bool _anchor, seq::DataType _value ): seq::type::Generic( seq::DataType::Type, _anchor ), value( _value ) {}

const seq::DataType seq::type::Type::getType() {
    return this->value;
}

seq::type::VMCall::VMCall( bool _anchor, seq::type::VMCall::CallType _value ): seq::type::Generic( seq::DataType::VMCall, _anchor ), value( _value ) {}

const seq::type::VMCall::CallType seq::type::VMCall::getCall() {
    return this->value;
}

seq::type::Name::Name( bool _anchor, bool _define, string _name ): seq::type::Generic( seq::DataType::Name, _anchor ), define( _define ), name( _name ) {}

const bool seq::type::Name::getDefine() {
    return this->define;
}

seq::string& seq::type::Name::getName() {
    return this->name;
}

seq::type::Function::Function( bool _anchor, seq::BufferReader* _reader ): seq::type::Generic( seq::DataType::Func, _anchor ), reader( _reader ) {}

seq::type::Function::Function( const seq::type::Function& func ): seq::type::Generic( seq::DataType::Func, func.anchor ), reader( new seq::BufferReader( *(func.reader) ) ) {}

seq::type::Function::~Function() {
    delete this->reader;
}

seq::BufferReader& seq::type::Function::getReader() {
    return *(this->reader);
}

seq::type::Expression::Expression( bool _anchor, seq::ExprOperator _op, seq::BufferReader* _left, seq::BufferReader* _right ): seq::type::Generic( seq::DataType::Expr, _anchor ), op( _op ), left( _left ), right( _right ) {}

seq::type::Expression::Expression( const seq::type::Expression& expr ): seq::type::Generic( seq::DataType::Expr, expr.anchor ), op( expr.op ), left( new seq::BufferReader( *(expr.left) ) ), right( new seq::BufferReader( *(expr.right) ) ) {}

seq::type::Expression::~Expression() {
    delete this->left;
    delete this->right;
}

const seq::ExprOperator seq::type::Expression::getOperator() {
    return this->op;
}

seq::BufferReader& seq::type::Expression::getLeftReader() {
    return *(this->left);
}

seq::BufferReader& seq::type::Expression::getRightReader() {
    return *(this->right);
}

seq::type::Stream::Stream( bool _anchor, byte _tags, BufferReader* _reader ): seq::type::Generic( seq::DataType::Stream, _anchor ), tags( _tags ), reader( _reader ) {}

seq::type::Stream::Stream( const seq::type::Stream& stream ): seq::type::Generic( seq::DataType::Stream, stream.anchor ), tags( stream.tags ), reader( new seq::BufferReader( *(stream.reader) ) ) {}

seq::type::Stream::~Stream() {
    delete this->reader;
}

const bool seq::type::Stream::machesTags( byte _tags ) {

    // execute stream on end ONLY if it has that tag
    if( _tags & SEQ_TAG_END ) return this->tags & SEQ_TAG_END;

    // stream don't have any tags
    if( this->tags == 0 ) return true;

     // check last & first tag
    if( this->tags & SEQ_TAG_FIRST ) return _tags & SEQ_TAG_FIRST;
    if( this->tags & SEQ_TAG_LAST ) return _tags & SEQ_TAG_LAST;

    // this stream is waiting for the end tag
    if( this->tags & SEQ_TAG_END ) return false;

    throw seq::InternalError( "Invalid Tag!" );
}

seq::BufferReader& seq::type::Stream::getReader() {
    return *(this->reader);
}

seq::type::Null::Null( bool _anchor ): seq::type::Generic( seq::DataType::Null, _anchor ) {}

seq::type::Flowc::Flowc( bool _anchor, const std::vector< seq::FlowCondition* > _blocks ): seq::type::Generic( seq::DataType::Flowc, _anchor ), conditions( _blocks ) {}

seq::type::Flowc::Flowc( const seq::type::Flowc& flowc ): seq::type::Generic( seq::DataType::Flowc, flowc.anchor ), conditions( seq::util::copyFlowConditions( flowc.conditions ) ) {}

seq::type::Flowc::~Flowc() {
    for( const auto& fc : this->conditions ) delete fc;
}

const std::vector< seq::FlowCondition* >& seq::type::Flowc::getConditions() {
    return this->conditions;
}

seq::Generic::Generic() {
	this->generic = new seq::type::Null( false );
}

seq::Generic::Generic( seq::type::Generic* _generic ) {
	this->generic = _generic;
}

seq::Generic::Generic( const seq::Generic& _generic ) {
	this->generic = seq::util::copyGeneric( _generic.generic );
}

seq::Generic::Generic( seq::Generic&& _generic ) {
	this->generic = std::move( _generic.generic );
	_generic.generic = nullptr;
}

seq::Generic::~Generic() {
	if( this->generic != nullptr ) delete this->generic;
}

seq::Generic& seq::Generic::operator= ( const Generic& generic ) {
	if( this != &generic ) {
		seq::type::Generic* g = seq::util::copyGeneric( generic.generic );
		delete this->generic;
		this->generic = g;
	}
	return *this;
}

seq::Generic& seq::Generic::operator= ( Generic&& generic ) noexcept {
	if( this != &generic ) {
		delete this->generic;
		this->generic = generic.generic;
		generic.generic = nullptr;
	}
	return *this;
}

const seq::DataType seq::Generic::getDataType() {
	return this->generic->getDataType();
}

bool seq::Generic::getAnchor() {
	return this->generic->getAnchor();
}

void seq::Generic::setAnchor( bool anchor ) {
	return this->generic->setAnchor( anchor );
}

seq::type::Generic* seq::Generic::getRaw() {
	return this->generic;
}

seq::type::Null& seq::Generic::Null() {
	return *(static_cast<seq::type::Null*>(this->generic));
}

seq::type::Stream& seq::Generic::Stream() {
	return *(static_cast<seq::type::Stream*>(this->generic));
}

seq::type::Flowc& seq::Generic::Flowc() {
	return *(static_cast<seq::type::Flowc*>(this->generic));
}

seq::type::Expression& seq::Generic::Expression() {
	return *(static_cast<seq::type::Expression*>(this->generic));
}

seq::type::Function& seq::Generic::Function() {
	return *(static_cast<seq::type::Function*>(this->generic));
}

seq::type::Name& seq::Generic::Name() {
	return *(static_cast<seq::type::Name*>(this->generic));
}

seq::type::VMCall& seq::Generic::VMCall() {
	return *(static_cast<seq::type::VMCall*>(this->generic));
}

seq::type::Type& seq::Generic::Type() {
	return *(static_cast<seq::type::Type*>(this->generic));
}

seq::type::String& seq::Generic::String() {
	return *(static_cast<seq::type::String*>(this->generic));
}

seq::type::Number& seq::Generic::Number() {
	return *(static_cast<seq::type::Number*>(this->generic));
}

seq::type::Arg& seq::Generic::Arg() {
	return *(static_cast<seq::type::Arg*>(this->generic));
}

seq::type::Bool& seq::Generic::Bool() {
	return *(static_cast<seq::type::Bool*>(this->generic));
}

seq::InternalError::InternalError( const std::string& error ) {
    this->error = "Internal Sequensa error occured: " + error;
}

const char* seq::InternalError::what() const throw() {
    return this->error.c_str();
}

seq::RuntimeError::RuntimeError( const std::string& error ) {
    this->error = error;
}

const char* seq::RuntimeError::what() const throw() {
    return this->error.c_str();
}

seq::ExecutorInterrupt::ExecutorInterrupt( const byte code ) {
    this->code = code;
}

const char* seq::ExecutorInterrupt::what() const throw() {
    return std::to_string( (int) this->code ).c_str();
}

byte seq::ExecutorInterrupt::getCode() {
    return this->code;
}

seq::CompilerError::CompilerError( const std::string& unexpected, const std::string& expected, const std::string& structure, int line ) {
    if( unexpected.empty() && expected.empty() ) this->error = "Unknown error"; else
    if( !unexpected.empty() && expected.empty() ) this->error = "Unexpected " + unexpected; else
    if( unexpected.empty() && !expected.empty() ) this->error = "Expected " + expected; else
    if( !unexpected.empty() && !expected.empty() ) this->error = "Unexpected " + unexpected + " (expected " + expected + ")";
    if( !structure.empty() ) this->error += " in " + structure;
    this->error += " at line: " + std::to_string(line);
}

const char* seq::CompilerError::what() const throw() {
    return this->error.c_str();
}

seq::ByteBuffer::ByteBuffer( byte* buffer, long length ) {
    this->pointer = buffer;
    this->length = length;
}

seq::BufferReader seq::ByteBuffer::getReader() {
    return seq::BufferReader( this->pointer, 0, this->length - 1 );
}

seq::BufferReader seq::ByteBuffer::getReader( long first, long last ) {
    if( first < 0 || last > (this->length - 1) || first > last ) throw seq::InternalError( "Invalid BufferReader range!" );
    return seq::BufferReader( this->pointer, first, last );
}

seq::BufferReader::BufferReader( byte* buffer, long first, long last ) {
    this->pointer = buffer;
    this->position = first - 1;
    this->first = first;
    this->last = last;
}

byte seq::BufferReader::peekByte() {
    if( this->position >= this->last ) return 0;
    return this->pointer[ this->position + 1 ];
}

byte seq::BufferReader::nextByte() {
    if( this->position >= this->last ) return 0;
    return this->pointer[ ++ this->position ];
}

bool seq::BufferReader::hasNext() {
    return ( this->position < this->last );
}

seq::TokenReader seq::BufferReader::next() {
    return seq::TokenReader( *this );
}

seq::BufferReader* seq::BufferReader::nextBlock( long length ) {
    if( length < 0 ) throw seq::InternalError( "Invalid block size!" );

    long new_pos = this->position + length;
    long old_pos = this->position + 1;

    this->position = std::min( new_pos, this->last );
    return new seq::BufferReader( this->pointer, old_pos, this->position );
}

long seq::BufferReader::nextInt() {

    byte head = this->nextByte();
    byte a = (head >> 4);
    long n = 0;

    if( a & (a - 1) ) throw seq::InternalError( "Invalid int header!" );

    if( a ) for( byte i = 0; i < a; i ++ ) {
        n |= ( (long) this->nextByte() ) << (i * 8);
    } else n = 0;

    return n;
}

seq::FileHeader seq::BufferReader::getHeader() {

    // validate file signature
    const byte s1[] = { this->nextByte(), this->nextByte(), this->nextByte(), this->nextByte() };
    const byte s2[] = { 's', 'q', 'c', 0 };
    if( !std::equal(std::begin(s1), std::end(s1), std::begin(s2)) ) throw InternalError( "Invalid header signature!" );

    // read sequensa version
    byte seq_major = this->nextByte();
    byte seq_minor = this->nextByte();
    byte seq_patch = this->nextByte();

    // read header data
    long n = 0;
    for( byte i = 0; i < 4; i ++ ) n |= ( (long) this->nextByte() ) << (i * 8);
    std::map< seq::string, seq::string > data;
    seq::string key;
    seq::string val;

    bool state = false;

    for( ; n > 0; n -- ) {
        byte b = this->nextByte();

        if( b ) {
            if( state ) {
                val.push_back( b );
            }else{
                key.push_back( b );
            }
        }else{
            if( state ) {
                state = false;
                data.insert( { key, val } );
                key.erase();
                val.erase();
            } else state = true;
        }
    }

    if( state ) throw InternalError( "Invalid header data, expected value!" );

    return seq::FileHeader( seq_major, seq_minor, seq_patch, data );
}

seq::Stream seq::BufferReader::readAll() {
    seq::Stream stream;
    while( this->hasNext() ) {
        TokenReader tr = this->next();
        stream.push_back( tr.getGeneric() );
    }

    return stream;
}

seq::ByteBuffer seq::BufferReader::getSubBuffer() {
	return seq::ByteBuffer( this->pointer + this->position + 1, this->last - this->position );
}

seq::TokenReader::TokenReader( seq::BufferReader& reader ): reader( reader ) {
    byte header = reader.nextByte();

    this->header = (byte) ( header & 0b01111111 );
    this->anchor = (bool) ( header & 0b10000000 );
    this->type = getDataType( this->header );

    switch( this->type ) {
        case seq::DataType::Bool: this->generic = seq::Generic( this->loadBool() ); break;
        case seq::DataType::Null: this->generic = seq::Generic( new seq::type::Null( this->anchor ) ); break;
        case seq::DataType::Number: this->generic = seq::Generic( this->loadNumber() ); break;
        case seq::DataType::String: this->generic = seq::Generic( this->loadString() ); break;
        case seq::DataType::Type: this->generic = seq::Generic( this->loadType() ); break;
        case seq::DataType::VMCall: this->generic = seq::Generic( this->loadCall() ); break;
        case seq::DataType::Arg: this->generic = seq::Generic( this->loadArg() ); break;
        case seq::DataType::Func: this->generic = seq::Generic( this->loadFunc() ); break;
        case seq::DataType::Expr: this->generic = seq::Generic( this->loadExpr() ); break;
        case seq::DataType::Name: this->generic = seq::Generic( this->loadName() ); break;
        case seq::DataType::Flowc: this->generic = seq::Generic( this->loadFlowc() ); break;
        case seq::DataType::Stream: this->generic = seq::Generic( this->loadStream() ); break;
        default: throw seq::InternalError( "Invalid data type!" );
    }
}

seq::Generic& seq::TokenReader::getGeneric() {
    return this->generic;
}

seq::DataType seq::TokenReader::getDataType( byte header ) {

    // its ugly but much faster than mapping it with switch-case
    if( header >= SEQ_MIN_OPCODE && header <= SEQ_MAX_OPCODE ) {
        byte id = header;
        if( header > 1 ) { // BLT BLF
            id --;
            if( header > 4 ) { // NUM INT
                id --;
                if( header > 12 ) { // VAR DEF
                    id--;
                }
            }
        }

        return (seq::DataType) id;
    }else{
        throw seq::InternalError( "Unknown head opcode! base: " + std::to_string( (int) header ) );
    }
}

seq::DataType seq::TokenReader::getDataType() {
    return this->type;
}

bool seq::TokenReader::isAnchored() {
    return this->anchor;
}

seq::type::Bool* seq::TokenReader::loadBool() {
    return new seq::type::Bool( this->anchor, this->header == (byte) seq::Opcode::BLT );
}

seq::type::Number* seq::TokenReader::loadNumber() {

    byte head = this->reader.nextByte();

    if( this->header == (byte) seq::Opcode::NUM ) {

        byte a = (head >> 4);
        byte b = (head & 0b00001111);
        unsigned long n = 0, d = 0;

        if( a & (a - 1) ) throw seq::InternalError( "Invalid numerator size! size: " + std::to_string( (int) a ) );
        if( b & (b - 1) ) throw seq::InternalError( "Invalid denominator size! size: " + std::to_string( (int) b ) );

        if( a ) for( byte i = 0; i < a; i ++ ) {
            n |= (( (long) this->reader.nextByte() ) << (i * 8));
        } else n = 0;

        if( b ) for( byte i = 0; i < b; i ++ ) {
            d |= (( (long) this->reader.nextByte() ) << (i * 8));
        } else d = 1;

        if( !d ) throw seq::InternalError( "Invalid denominator! size: " + std::to_string( (int) b ) + " value: " + std::to_string( (int) d ) );

        return new seq::type::Number( this->anchor, (n & (1ul << ((unsigned long) a * 8ul - 1ul))) ? -n : n, d );

    }else{

        return new seq::type::Number( this->anchor, (long) (signed char) head, 1 );

    }
}

seq::type::Arg* seq::TokenReader::loadArg() {
    return new seq::type::Arg( this->anchor, this->reader.nextByte() );
}

seq::type::String* seq::TokenReader::loadString() {
    seq::string str;
    while( true ) {
        byte b = this->reader.nextByte();
        if( b ) str.push_back( b ); else return new seq::type::String( this->isAnchored(), str.c_str() );
    }
}

seq::type::Type* seq::TokenReader::loadType() {
    byte b = this->reader.nextByte();
    if( b > SEQ_MAX_DATA_TYPE || b < SEQ_MIN_DATA_TYPE ) throw seq::InternalError( "Invalid data type!" );
    return new seq::type::Type( this->anchor, (seq::DataType) b );
}

seq::type::VMCall* seq::TokenReader::loadCall() {
    byte b = this->reader.nextByte();
    if( b > SEQ_MAX_CALL_TYPE && b < SEQ_MIN_CALL_TYPE ) throw seq::InternalError( "Invalid call type!" );
    return new seq::type::VMCall( this->anchor, (seq::type::VMCall::CallType) b );
}

seq::type::Name* seq::TokenReader::loadName() {
    seq::string str;
    for( byte i = 0; true; i ++ ) {
        byte b = this->reader.nextByte();
        if( !(std::isalnum((int) b) || (b == '_'_b) || (b == ':'_b) || (b == '\0'_b)) ) throw seq::InternalError( "Invalid char in name! code: " + std::to_string( (int) b ) );
        if( i > 128 ) throw seq::InternalError( "Too long name!" );
        if( b ) str.push_back( b ); else break;
    }

    return new seq::type::Name( this->anchor, this->header == (byte) seq::Opcode::DEF, str );
}

seq::type::Function* seq::TokenReader::loadFunc() {
    long length = this->reader.nextInt();
    if( !length ) throw seq::InternalError( "Invalid function size!" );
    return new seq::type::Function( this->anchor, this->reader.nextBlock( length ) );
}

seq::type::Expression* seq::TokenReader::loadExpr() {

    if( this->reader.peekByte() < SEQ_MIN_OPERATOR || this->reader.peekByte() > SEQ_MAX_OPERATOR ) {
        throw seq::InternalError( "Invalid operator!" );
    }

    seq::ExprOperator op = (seq::ExprOperator) this->reader.nextByte();

    byte head = this->reader.nextByte();
    byte a = (head >> 4);
    byte b = (head & 0b00001111);
    long l = 0, r = 0;

    if( a & (a - 1) ) throw seq::InternalError( "Invalid expression (left) size!" );
    if( b & (b - 1) ) throw seq::InternalError( "Invalid expression (right) size!" );

    if( a ) for( byte i = 0; i < a; i ++ ) {
        l |= ( (long) this->reader.nextByte() ) << (i * 8);
    }

    if( b ) for( byte i = 0; i < a; i ++ ) {
        r |= ( (long) this->reader.nextByte() ) << (i * 8);
    }

    if( !(l && r) ) throw seq::InternalError( "Invalid expression size!" );

    return new seq::type::Expression( this->anchor, op, this->reader.nextBlock( l ), this->reader.nextBlock( r ) );
}

seq::type::Flowc* seq::TokenReader::loadFlowc() {

    auto blocks = std::vector<seq::FlowCondition*>();
    byte block_count = this->reader.nextByte();
    for( byte i = 0; i < block_count; i ++ ) {

        long length = this->reader.nextInt();
        if( !length ) throw seq::InternalError( "Invalid flowc size!" );
        seq::BufferReader* br = this->reader.nextBlock( length );
        seq::TokenReader tr = br->next();

        if( br->hasNext() ) {
            blocks.push_back( new seq::FlowCondition( seq::FlowCondition::Type::Range, tr.getGeneric(), br->next().getGeneric() ) );
        }else{
            if( tr.getDataType() == seq::DataType::Type ) {
                blocks.push_back( new seq::FlowCondition( seq::FlowCondition::Type::Type, tr.getGeneric(), seq::Generic() ) );
            }else{
                blocks.push_back( new seq::FlowCondition( seq::FlowCondition::Type::Value, tr.getGeneric(), seq::Generic() ) );
            }
        }

        delete br;

    }

    return new seq::type::Flowc( this->anchor, blocks );
}

seq::type::Stream* seq::TokenReader::loadStream() {
    byte tags = this->reader.nextByte();
    long length = this->reader.nextInt();
    if( !length ) throw seq::InternalError( "Invalid stream size!" );
    return new seq::type::Stream( this->anchor, tags, this->reader.nextBlock( length ) );
}

seq::StackLevel::StackLevel( seq::Generic arg ) {
    this->arg = arg;
}

seq::StackLevel::StackLevel( seq::StackLevel&& level ) {
    this->arg = std::move( level.arg );
    this->vars = std::move( level.vars );
}

seq::Generic seq::StackLevel::getArg() {
    return seq::Generic( this->arg );
}

seq::Stream seq::StackLevel::getVar( seq::string& name, bool anchor ) {
    seq::Stream ret;
    for( auto& g : this->vars.at( name ) ) {
        seq::Generic ng = seq::Generic( g );
        ng.setAnchor( anchor );
        ret.push_back( ng );
    }

    return ret;
}

void seq::StackLevel::setVar( seq::string& name, seq::Stream value ) {
    this->vars[ name ] = value;
}

void seq::StackLevel::setArg( seq::Generic _arg ) {
    this->arg = _arg;
}

seq::FlowCondition::FlowCondition( seq::FlowCondition::Type _type, seq::Generic _a, seq::Generic _b ): type( _type ), a( _a ), b( _b ) {}

bool seq::FlowCondition::validate( seq::Generic arg ) {

    switch( this->type ) {
        case seq::FlowCondition::Type::Value: {
        		auto type = arg.getDataType();
        		if( type == this->a.getDataType() ) {
        			switch( type ) {
        				case seq::DataType::Null:
        					return true;

        				case seq::DataType::Bool:
        					return this->a.Bool().getBool() == arg.Bool().getBool();

        				case seq::DataType::Number:
        					return this->a.Number().getDouble() == arg.Number().getDouble();

        				default:
        					return false;
        			}
        		}
        	}
            return false;

        case seq::FlowCondition::Type::Type:
            return (arg.getDataType() == this->a.Type().getType());

        case seq::FlowCondition::Type::Range:
        	if( arg.getDataType() == seq::DataType::Number ) {
        		double av = this->a.Number().getDouble();
        		double bv = this->b.Number().getDouble();
        		double val = arg.Number().getDouble();
        		return val < bv && val > av;
        	}
        	return false;
    }

    return false;
}

seq::CommandResult::CommandResult( seq::CommandResult::ResultType _stt, seq::Stream _acc ): stt( _stt ), acc( _acc ) {}

void seq::Executor::inject( seq::string name, seq::type::Native native ) {
	this->natives[ name ] = native;
}

seq::StackLevel* seq::Executor::getLevel( int level ) {
	try{
		return &(this->stack.at( level ));
	}catch( std::out_of_range& ex ) {
		return nullptr;
	}
}

seq::StackLevel* seq::Executor::getTopLevel() {
	return &(this->stack.back());
}

void seq::Executor::reset() {
    this->natives.clear();
}

std::string seq::Executor::getResultString() {
    return seq::util::toStdString( seq::util::stringCast( this->getResult() ).String().getString() );
}

seq::Generic seq::Executor::getResult() {
    return this->result.at(0);
}

seq::Stream& seq::Executor::getResults() {
    return this->result;
}

void seq::Executor::execute( seq::ByteBuffer bb, seq::Stream args ) {
	static seq::Stream exitStream = seq::Stream { seq::Generic( new seq::type::Null( false ) ) };

	try{
		this->executeFunction( bb.getReader(), args );
		this->exit( exitStream, 0 );
	}catch( seq::ExecutorInterrupt& ex ) {
		// this->result set by the this->exit method
	}
}

void seq::Executor::exit( seq::Stream& stream, byte code ) {
	if( stream.size() == 0 ) {
		throw seq::InternalError( "Unable to exit without arguments!" );
	}

	// stop program execution
    this->result = stream;
    throw seq::ExecutorInterrupt( code );
}

seq::Stream seq::Executor::executeFunction( seq::BufferReader fbr, seq::Stream& input_stream ) {

    // push new stack into stack array
    this->stack.push_back( seq::StackLevel( seq::Generic() ) );

    // accumulator of all returned entities
    seq::Stream acc;

    // execute scope for each input_stream element
    for ( long i = 0; i <= (long) input_stream.size(); i ++ ) {

        const long size = input_stream.size();
        const byte tags = seq::util::packTags( i, size );
        seq::BufferReader br = fbr;

        // set current stack argument
        this->getTopLevel()->setArg( (i == size) ? seq::Generic( new seq::type::Null( false ) ) : input_stream.at(i) );

        // iterate over function code
        while( br.hasNext() ) {

            // run next command
            seq::TokenReader tk = br.next();
            seq::CommandResult cr = this->executeCommand( &tk, tags );

            // check state
            switch( cr.stt ) {

                case seq::CommandResult::ResultType::Return:
                	// insert returned data to function output stream
                    acc.insert(acc.end(), cr.acc.begin(), cr.acc.end());
                    break;

                case seq::CommandResult::ResultType::Break:
                	// exit scope
                    goto exit;
                    break;

                case seq::CommandResult::ResultType::Exit:
                	// stop program execution (by throwing exception)
                    this->exit( cr.acc, 0 );
                    break;

                case seq::CommandResult::ResultType::Final:
                	// exit scope and return value
                	acc.insert(acc.end(), cr.acc.begin(), cr.acc.end());
                	goto exit;
                	break;

                case seq::CommandResult::ResultType::Again:
                	// add returned arguments to CURRENT input stream
                    if( i == size ) throw RuntimeError( "Native function 'again' can not be called from 'end' taged stream!" );
                    input_stream.insert( input_stream.begin() + i + 1, cr.acc.begin(), cr.acc.end() );
                    break;

                default:
                	break;

            }
        }
    }

    exit:

    // pop scope from stack
    this->stack.pop_back();

    // return all accumulated entities
    return acc;
}

seq::CommandResult seq::Executor::executeCommand( seq::TokenReader* tr, byte tags ) {

	// functions can only contain streams
    if( tr->getDataType() == seq::DataType::Stream ) {

    	// execute stream if stream tags match current state
    	auto generic = tr->getGeneric();
        auto& stream = generic.Stream();

        if( stream.machesTags( tags ) ) {
        	seq::Stream s = stream.getReader().readAll();
            return this->executeStream( s );
        }else{
            return CommandResult( seq::CommandResult::ResultType::None, seq::Stream() );
        }

    }

    throw seq::InternalError( "Invalid command in function!" );
}

seq::CommandResult seq::Executor::executeStream( seq::Stream& gs ) {

    seq::Stream acc;

    // iterate over stream entities
    for( int i = gs.size() - 1; i >= 0; i -- ) {

        seq::Generic g = gs.at( i );
        seq::DataType t = g.getDataType();

        // if type is unsolid compute real value
        if( t == seq::DataType::Expr || t == seq::DataType::Arg ) {
            g = this->executeExpr( g );
            t = g.getDataType();
        }

        // execute anchored entities
        if( g.getAnchor() ) {

            // handle native function "emit"
            if( t == seq::DataType::VMCall && g.VMCall().getCall() == seq::type::VMCall::CallType::Emit ) {
                if( acc.size() == 0 ) {
                    acc.push_back( seq::Generic( new seq::type::Null( false ) ) );
                }

                continue;
            }

            // if acc is empty there is nothing to execute - skip
            if( acc.size() == 0 ) {
                continue;
            }

            // if entity is a VM Call other than "emit"
            if( t == seq::DataType::VMCall ) {

            	// get VMCall type and using a hacky way cast it to ResultType, then return
                auto stt = (seq::CommandResult::ResultType) (byte) g.VMCall().getCall();
                return CommandResult( stt, acc );

            }else{

            	// execute anchor and save result in acc
            	acc = this->executeAnchor( g, acc );
                continue;

            }
        }

        // if entity is a variable
        if( t == seq::DataType::Name ) {
            auto& name = g.Name();

            if( name.getDefine() ) { // define variable (set)

                this->stack.back().setVar( name.getName(), acc );
                acc.clear();

            }else{ // read variable from stack

                auto tmp = this->resolveName( name.getName(), name.getAnchor() );

                // append tmp to acc
                acc.reserve( acc.size() + tmp.size() );
                acc.insert( acc.end(), tmp.begin(), tmp.end() );
            }

            continue;
        }

        // if entity is a simple, solid value add it to acc
        acc.insert( acc.begin(), g );
    }

    return CommandResult( seq::CommandResult::ResultType::None, acc );
}

seq::Stream seq::Executor::executeAnchor( seq::Generic entity, seq::Stream& input_stream ) {

    seq::DataType type = entity.getDataType();

    // if given entity is a variable
    if( type == seq::DataType::Name ) {

        seq::type::Name& name = entity.Name();

        // test if name refers to native function, and if so execute it
        try{
            return this->natives.at( name.getName() )( input_stream );
		} catch (std::out_of_range &ignore) {

			// if it isn't native, try finding it on the stack
			seq::Stream s = this->resolveName( name.getName(), true );
			s.insert( s.end(), input_stream.begin(), input_stream.end() );
			return this->executeStream( s ).acc;

		}

    }

    // execute anchored function
    if( type == seq::DataType::Func ) {
        return this->executeFunction( entity.Function().getReader(), input_stream );
    }

    // execute anchored flowc
    if( type == seq::DataType::Flowc ) {
        return this->executeFlowc( entity.Flowc().getConditions(), input_stream );
    }

    // if not any of the above, simply cast args to anchor
    seq::Stream output_stream;
    for( auto g : input_stream ) {
        output_stream.push_back( this->executeCast( entity, g ) );
    }
    return output_stream;

}

seq::Generic seq::Executor::executeExprPair( seq::Generic left, seq::Generic right, seq::ExprOperator op, bool anchor ) {

	if( left.getDataType() == seq::DataType::Expr || left.getDataType() == seq::DataType::Arg ) left = this->executeExpr(left);
	if( right.getDataType() == seq::DataType::Expr || right.getDataType() == seq::DataType::Arg ) right = this->executeExpr(right);

	// not and binary not use only one argument (right)
	if( op != seq::ExprOperator::Not && op != seq::ExprOperator::BinaryNot ) {
		if( left.getDataType() != right.getDataType() || left.getDataType() == seq::DataType::Null ) {
			return seq::Generic( new seq::type::Null(anchor) );
		}

	}

	seq::DataType type = right.getDataType();

	typedef seq::Generic G;
	auto str = [] ( G g ) -> seq::type::String* { return (seq::type::String*) g.getRaw(); };
	auto num = [] ( G g ) -> seq::type::Number* { return (seq::type::Number*) g.getRaw(); };

	static const std::array<std::array<std::function<seq::type::Generic*(bool,seq::Generic,seq::Generic)>,2>,SEQ_MAX_OPERATOR> lambdas = {{
			{ // Less
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(anchor, num(a)->getDouble() < num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(anchor); },
			},
			{ // Greater
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(a)->getDouble() > num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Equal
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(a)->getDouble() == num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, str(a)->getString() == str(b)->getString()); },
			},
			{ // NotEqual
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(a)->getDouble() != num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, str(a)->getString() != str(b)->getString()); },
			},
			{ // NotGreater
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(a)->getDouble() <= num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // NotLess
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(a)->getDouble() >= num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // And
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(a)->getLong() != 0 && num(b)->getLong() != 0); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Or
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(a)->getLong() != 0 || num(b)->getLong() != 0); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Xor
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, (num(a)->getLong() != 0) != (num(b)->getLong() != 0)); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Not
					[&] ( bool f, G a, G b ) { return new seq::type::Bool(f, num(b)->getLong() == 0); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Multiplication
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getDouble() * num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Division
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getDouble() / num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Addition
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getDouble() + num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::String(f, (str(a)->getString() + str(b)->getString()).c_str() ); },
			},
			{ // Subtraction
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getDouble() - num(b)->getDouble()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Modulo
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getLong() % num(b)->getLong()); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // Power
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, std::pow( num(a)->getDouble(), num(b)->getDouble() )); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // BinaryAnd
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getLong() & num(b)->getLong() ); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // BinaryOr
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getLong() | num(b)->getLong() ); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // BinaryXor
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, num(a)->getLong() ^ num(b)->getLong() ); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			},
			{ // BinaryNot
					[&] ( bool f, G a, G b ) { return new seq::type::Number(f, ~ num(b)->getLong() ); },
					[&] ( bool f, G a, G b ) { return new seq::type::Null(f); },
			}
	}};

	if( type == seq::DataType::Number ) {

		return seq::Generic( lambdas.at( ((byte) op) - 1 ).at( 0 )( anchor, left, right ) );

	}else if( type == seq::DataType::Bool ) {

		G lb = seq::Generic( new seq::type::Number( false, (float) (left.Bool().getBool() ? 1 : 0) ) );
		G rb = seq::Generic( new seq::type::Number( false, (float) (right.Bool().getBool() ? 1 : 0) ) );
		seq::type::Generic* r = lambdas.at( ((byte) op) - 1 ).at( 0 )( anchor, lb, rb );

		if( r->getDataType() == seq::DataType::Bool ) {
			return seq::Generic( r );
		}else{
			seq::type::Number* r_num = (seq::type::Number*) r;
			bool val = r_num->getLong() != 0;
			delete r_num;

			return seq::Generic( new seq::type::Bool( anchor, val ) );
		}

	}else if( type == seq::DataType::String ) {

		return seq::Generic( lambdas.at( ((byte) op) - 1 ).at( 1 )( anchor, left, right ) );

	}

	throw seq::InternalError( "Invalid operands!" );

}

seq::Generic seq::Executor::executeExpr( seq::Generic entity ) {

    seq::DataType type = entity.getDataType();
    bool anchor = entity.getAnchor();

    if( type == seq::DataType::Expr ) {
        auto& expr = entity.Expression();

        seq::Generic left = expr.getLeftReader().next().getGeneric();
        seq::Generic right = expr.getRightReader().next().getGeneric();
        seq::ExprOperator op = expr.getOperator();

        return this->executeExprPair( left, right, op, anchor );
    }

    if( type == seq::DataType::Arg ) {
        auto& arg = entity.Arg();

        long s = (long) this->stack.size() - 1L - (long) arg.getLevel();

        if( s < 0 ) {
            return seq::Generic( new seq::type::Null( anchor ) );
        }

        auto ret = this->stack.at( s ).getArg();
        ret.setAnchor(anchor);
        return ret;
    }

    return entity;

}

seq::Stream seq::Executor::resolveName( seq::string& name, bool anchor ) {

    for( int i = (int) this->stack.size() - 1; i >= 0; i -- ) {
        try{
            return this->stack.at(i).getVar( name, anchor );
		} catch (std::out_of_range &err) {
            continue;
        }
    }

    return seq::Stream();

}

seq::Stream seq::Executor::executeFlowc( std::vector<seq::FlowCondition*> fcs, seq::Stream& input_stream ) {

    seq::Stream acc;

    for( seq::Generic& arg : input_stream ) {
        for( seq::FlowCondition* fc : fcs ) {
            if( fc->validate( arg ) ) {
                acc.push_back( arg );
            }
        }
    }

    return acc;
}

seq::Generic seq::Executor::executeCast( seq::Generic cast, seq::Generic arg ) {

    switch( cast.getDataType() ) {

        // simple value casts:
        case seq::DataType::Bool:
        case seq::DataType::Null:
        case seq::DataType::Number:
        case seq::DataType::String:
        case seq::DataType::VMCall:
        case seq::DataType::Flowc:
        case seq::DataType::Func:
        	cast.setAnchor(false);
            return cast;

        // type cast:
        case seq::DataType::Type: {
            switch( cast.Type().getType() ) {

                case seq::DataType::Bool:
                    return seq::util::boolCast( arg );

                case seq::DataType::Number:
                    return seq::util::numberCast( arg );

                case seq::DataType::String:
                    return seq::util::stringCast( arg );

                default: throw seq::InternalError( "Invalid cast!" );
            }
        }

        // invalid casts: (stream, name, expr, arg)
        default: throw seq::InternalError( "Invalid cast!" );

    }

}

#ifndef SEQ_EXCLUDE_COMPILER
seq::Compiler::Token::Token( unsigned int _line, long _data, bool _anchor, Category _category, seq::string& _raw, seq::string& _clean ): line( _line ), data( _data ), anchor( _anchor ),  category( _category ), raw( _raw ), clean( _clean ) {}

seq::Compiler::Token::Token( const Token& token ): line( token.line ), data( token.data ), anchor( token.anchor ), category( token.category ), raw( token.raw ), clean( token.clean ) {}

seq::Compiler::Token::Token( seq::Compiler::Token&& token ): line( std::move( token.line ) ), data( std::move( token.data ) ), anchor( std::move( token.anchor ) ), category( std::move( token.category ) ), raw( std::move( token.raw ) ), clean( std::move( token.clean ) ) {}

const unsigned int seq::Compiler::Token::getLine() {
	return this->line;
}

const seq::Compiler::Token::Category seq::Compiler::Token::getCategory() {
	return this->category;
}

const seq::string& seq::Compiler::Token::getRaw() {
	return this->raw;
}

const seq::string& seq::Compiler::Token::getClean() {
	return this->clean;
}

const long seq::Compiler::Token::getData() {
	return this->data;
}

const bool seq::Compiler::Token::getAnchor() {
	return this->anchor;
}

const bool seq::Compiler::Token::isPrimitive() {

	if( this->category == seq::Compiler::Token::Category::Arg ) return true;
	if( this->category == seq::Compiler::Token::Category::Null ) return true;
	if( this->category == seq::Compiler::Token::Category::Bool ) return true;
	if( this->category == seq::Compiler::Token::Category::Number ) return true;
	if( this->category == seq::Compiler::Token::Category::Type ) return true;
	if( this->category == seq::Compiler::Token::Category::String ) return true;
	if( this->category == seq::Compiler::Token::Category::VMC ) return true;

	return false;
}

std::string seq::Compiler::Token::toString() {
	std::string catstr = (const char *[]) {
		"Tag",
		"Set",
		"Load",
		"Bool",
		"Null",
		"Type",
		"Name",
		"Number",
		"Stream",
		"Operator",
		"String",
		"FuncBracket",
		"FlowBracket",
		"MathBracket",
		"Comma",
		"Colon",
		"Arg",
		"VMCall"
	}[ (byte) getCategory() ];
	return catstr + "::" + seq::util::toStdString( this->getRaw() );
}

std::vector<byte> seq::Compiler::compile( seq::string code, std::vector<seq::string>* headerData ) {
	auto tokens = seq::Compiler::tokenize( code );

	// skip empty files
	if( tokens.empty() ) return std::vector<byte>();

	int offset = seq::Compiler::extractHeaderData(tokens, headerData);
	auto buffer = seq::Compiler::assembleFunction( tokens, offset, tokens.size(), true );

	// get rid of the first function opcode
	int functionOffset = (buffer.at(1) >> 4) + 2;

	// if that is NOT the case something weird has happened/the file is empty
	// but we will pretend that everything is OK and just skip this step
	if( functionOffset < (long) buffer.size() ) {
		buffer.erase( buffer.begin(), buffer.begin() + functionOffset );
	}

	return buffer;
}

std::vector<seq::Compiler::Token> seq::Compiler::tokenize( seq::string code ) {

	enum struct State {
		Start,
		Comment,
		String,
		Escape,
		Name,
		Name2,
		Number,
		Number2,
		Arg
	};

	const static std::vector<seq::string> long_operators = { "!="_b, ">="_b, "!>"_b, "<="_b, "!<"_b, "&&"_b, "||"_b, "^^"_b, "**"_b };
	const static std::vector<byte> short_operators = { '+'_b, '-'_b, '/'_b, '%'_b, '*'_b, '>'_b, '<'_b, '='_b, '&'_b, '|'_b, '^'_b, '~'_b };
	const static std::vector<seq::string> brackets = { "{ "_b, "} "_b, "#{"_b, "[ "_b, "] "_b, "#["_b, "( "_b, ") "_b, "#("_b };

	State state = State::Start;
	seq::string token;
	std::vector<seq::Compiler::Token> tokens;
	unsigned int line = 1;
	int roundBrackets = 0;
	int curlyBrackets = 0;
	int squareBrackets = 0;

	auto next = [&] () -> void {
		if( !token.empty() ) {
			tokens.push_back( seq::Compiler::construct( token , line ) );
			token = (byte*) "";
		}
	};

	auto isLetter = [&] (byte chr) -> bool {
		return std::isalpha( chr ) || chr == '_'_b;
	};

	auto isNumber = [&] (byte chr) -> bool {
		return std::isdigit( chr );
	};

	auto bind = [] (byte a, byte b) -> seq::string {
		seq::string str;
		str += a;
		str += b;
		return str;
	};

	auto updateBrackes = [&] (byte chr) {
		switch( chr ) {
			case '{'_b: curlyBrackets ++; break;
			case '}'_b: curlyBrackets --; break;
			case '['_b: squareBrackets ++; break;
			case ']'_b: squareBrackets --; break;
			case '('_b: roundBrackets ++; break;
			case ')'_b: roundBrackets --; break;
			default: throw seq::InternalError( "Invalid lambda argument!" );
		}

		if( curlyBrackets < 0 ) throw seq::CompilerError( "'}'", "", "", line );
		if( roundBrackets < 0 ) throw seq::CompilerError( "')'", "", "", line );
		if( squareBrackets != 0 && squareBrackets != 1 ) {
			std::string msg = "'";
			msg += (char) chr;
			msg += '\'';

			throw seq::CompilerError( msg, "", "", line );
		}

	};

	const int size = code.size();
	for( int i = 0; i < size; i ++ ) {

		const byte c = code.at(i);
		const byte n = i + 1 < size ? code.at(i + 1) : '\0'_b;

		// keep line number up-to-date
		if( c == '\n'_b ) {
			if( state == State::String ) throw CompilerError( "end of line", "end of string", "", line );
			next();
			line ++;
			continue;
		}

		for( bool flag = true; flag; ) {

			flag = false;

			switch( state ) {

				case State::Start: {
					if( !token.empty() ) throw seq::InternalError( "Invalid tokenizer state!" );
					if( c == '/'_b && n == '/'_b ) { i ++; state = State::Comment; break; }
					if( c == '"'_b ) { state = State::String; token += c; break; }
					if( c == ' '_b || c == '\n'_b || c == '\t'_b ) { break; }
					if( (c == '#'_b && isLetter( n )) || isLetter( c ) ) { state = State::Name; token += c; break; }
					if( (c == '<'_b && n == '<'_b) || (c == '>'_b && n == '>'_b) ) { token += c; token += n; i ++; next(); break; }
					if( (c == '#'_b && isNumber( n )) || isNumber( c ) ) { state = State::Number; token += c; break; }
					if( std::find(long_operators.begin(), long_operators.end(), bind( c, n )) != long_operators.end() ) { token += c; token += n; i ++; next(); break; }
					if( std::find(short_operators.begin(), short_operators.end(), c) != short_operators.end() ) { token += c; next(); break; }
					if( c == '!'_b ) { token += "null"_b; next(); token += c; next(); break; }
					if( (c == '#'_b && n == '@'_b) || c == '@'_b ) { state = State::Arg; token += c; break; }
					if( std::find(brackets.begin(), brackets.end(), bind( c, ' ' ) ) != brackets.end() ) { token += c; updateBrackes( c ); next(); break; }
					if( std::find(brackets.begin(), brackets.end(), bind( c, n ) ) != brackets.end() ) { token += c; token += n; updateBrackes( n ); i ++; next(); break; }
					if( c == ','_b || c == ':'_b ) { token += c; next(); break; }

					std::string msg = "char: '";
					msg += (char) c;
					msg += '\'';
					throw CompilerError( msg, "", "", line );
				}

				case State::Comment:
					if( (c == '/'_b && n == '/'_b) || c == '\n'_b ) {
						i ++;
						state = State::Start;
					}
					break;

				case State::String:
					if( c == '\\'_b ) {
						state = State::Escape;
					}else if( c == '"'_b ) {
						token += '"'_b;
						state = State::Start;
						next();
					}else{
						token += c;
					}
					break;

				case State::Escape:
					switch( c ) {
						case 'n'_b: token += '\n'_b; break;
						case 't'_b: token += '\t'_b; break;
						case 'r'_b: token += '\r'_b; break;
						case '\\'_b: token += '\\'_b; break;
						case '"'_b: token += '"'_b; break;
						default: throw CompilerError( std::string("char '") + (char) c + std::string("'"), "escape code (n, t, r, \\ or \")", "string", line );
					}
					state = State::String;
					break;

				case State::Name: // or Tag
					if( isLetter( c ) ) token += c; else {

						if( c == ':'_b ) {
							token += c;
							state = State::Name2;
							break;
						}

						if( c == ';'_b ) {
							token += c;
						}else{
							flag = true;
						}

						state = State::Start;
						next();
					}
					break;

				case State::Name2:
					if( c == ':'_b ) {
						state = State::Start;
						flag = true;
						next();
					}else{
						state = State::Name;
						flag = true;
					}
					break;

				case State::Number:
					if( isNumber(c) ) token += c; else {
						if( c == '.'_b ) {
							token += c;
							state = State::Number2;
						} else {
							state = State::Start;
							flag = true;
							next();
						}
					}
					break;

				case State::Number2:
					if( isNumber(c) ) token += c; else {
						state = State::Start;
						flag = true;
						next();
					}
					break;

				case State::Arg:
					if( c == '@'_b ) token += c; else {
						state = State::Start;
						flag = true;
						next();
					}
					break;

			}

		}

	}

	if( state == State::String ) throw CompilerError( "end of input", "end of string", "", line );
	if( curlyBrackets != 0 ) throw CompilerError( "end of input", "curly bracket", "", line );
	if( roundBrackets != 0 ) throw CompilerError( "end of input", "round bracket", "", line );
	if( squareBrackets != 0 ) throw CompilerError( "end of input", "square bracket", "", line );

	next(); // if some token is still left, add it.
	return tokens;
}

seq::Compiler::Token seq::Compiler::construct( seq::string raw, unsigned int line ) {

	// setup regex'es and string vectors
	const static std::vector<std::string> operators = { "+", "-", "/", "*", "**", "%", ">", "<", "=", "!=", ">=", "!>", "<=", "!<", "&&", "||", "^^", "&", "|", "^", "!" };
	const static std::vector<byte> operator_weights = {  17,  17,  16,  16,   15,  16,  18,  18,  18,   18,   18,   18,   18,   18,   19,   19,   19,  17,  17,  17,  15 };
	const static std::regex regex_arg("^@+$");
	const static std::regex regex_name("^[a-zA-Z_]{1}[a-zA-Z_0-9]*(:[a-zA-Z_0-9]+)*$");
	const static std::regex regex_number_1("^\\d+.\\d+$");
	const static std::regex regex_number_2("^\\d+$");

	const bool anchor = raw.front() == '#'_b;
	seq::string clean = anchor ? raw.substr(1) : raw;

	auto make = [&] ( seq::Compiler::Token::Category c, long data ) -> seq::Compiler::Token {
		return seq::Compiler::Token( line, data, anchor, c, raw, clean );
	};

	auto callTypeFromString = [] ( seq::string str ) -> long {
		if( str == "return"_b ) return (byte) seq::type::VMCall::CallType::Return;
		if( str == "break"_b ) return (byte) seq::type::VMCall::CallType::Break;
		if( str == "exit"_b ) return (byte) seq::type::VMCall::CallType::Exit;
		if( str == "again"_b ) return (byte) seq::type::VMCall::CallType::Again;
		if( str == "emit"_b ) return (byte) seq::type::VMCall::CallType::Emit;
		if( str == "final"_b ) return (byte) seq::type::VMCall::CallType::Final;
		return 0;
	};

	auto dataTypeFromString = [] ( seq::string str ) -> long {
		if( str == "number"_b ) return (byte) seq::DataType::Number;
		if( str == "bool"_b ) return (byte) seq::DataType::Bool;
		if( str == "string"_b ) return (byte) seq::DataType::String;
		if( str == "type"_b ) return (byte) seq::DataType::Type;
		return 0;
	};

	auto tagTypeFromString = [] ( seq::string str ) -> long {
		if( str == "first;"_b ) return SEQ_TAG_FIRST;
		if( str == "last;"_b ) return SEQ_TAG_LAST;
		if( str == "end;"_b ) return SEQ_TAG_END;
		return 0;
	};

	auto operatorDataFromString = [] ( seq::string str, unsigned int weight ) -> long {
		seq::ExprOperator op;
		if( str == "<"_b ) op = seq::ExprOperator::Less; else
		if( str == ">"_b ) op = seq::ExprOperator::Greater; else
		if( str == "="_b ) op = seq::ExprOperator::Equal; else
		if( str == "!="_b ) op = seq::ExprOperator::NotEqual; else
		if( str == "!>"_b ) op = seq::ExprOperator::NotGreater; else
		if( str == "<="_b ) op = seq::ExprOperator::NotGreater; else
		if( str == "!<"_b ) op = seq::ExprOperator::NotLess; else
		if( str == ">="_b ) op = seq::ExprOperator::NotLess; else
		if( str == "&&"_b ) op = seq::ExprOperator::And; else
		if( str == "||"_b ) op = seq::ExprOperator::Or; else
		if( str == "^^"_b ) op = seq::ExprOperator::Xor; else
		if( str == "!"_b ) op = seq::ExprOperator::Not; else
		if( str == "*"_b ) op = seq::ExprOperator::Multiplication; else
		if( str == "/"_b ) op = seq::ExprOperator::Division; else
		if( str == "+"_b ) op = seq::ExprOperator::Addition; else
		if( str == "-"_b ) op = seq::ExprOperator::Subtraction; else
		if( str == "%"_b ) op = seq::ExprOperator::Modulo; else
		if( str == "**"_b ) op = seq::ExprOperator::Power; else
		if( str == "&"_b ) op = seq::ExprOperator::BinaryAnd; else
		if( str == "|"_b ) op = seq::ExprOperator::BinaryOr; else
		if( str == "^"_b ) op = seq::ExprOperator::BinaryXor; else
		if( str == "~"_b ) op = seq::ExprOperator::BinaryNot;

		long operatorCode = (long) op;
		return weight | (operatorCode << 8);
	};


	{ // categorize and create new token
		auto callType = callTypeFromString( clean );
		if( callType != 0 ) return make( seq::Compiler::Token::Category::VMC, callType );

		auto dataType = dataTypeFromString( clean );
		if( dataType != 0 ) return make( seq::Compiler::Token::Category::Type, dataType );

		auto tagType = tagTypeFromString( raw );
		if( tagType != 0 ) return make( seq::Compiler::Token::Category::Tag, tagType );

		if( raw == "set"_b ) return make( seq::Compiler::Token::Category::Set, 0 );
		if( raw == "load"_b ) return make( seq::Compiler::Token::Category::Load, 0 );
		if( clean == "true"_b || clean == "false"_b ) return make( seq::Compiler::Token::Category::Bool, clean == "true"_b ? 1 : 0 );
		if( clean == "null"_b ) return make( seq::Compiler::Token::Category::Null, 0 );
		if( clean == "number"_b || clean == "bool"_b || clean == "string"_b || clean == "type"_b ) return make( seq::Compiler::Token::Category::Type, dataTypeFromString( clean ) );
		if( std::regex_search( seq::util::toStdString(clean), regex_name) ) return make( seq::Compiler::Token::Category::Name, 0 );
		if( std::regex_search( seq::util::toStdString(clean), regex_number_1) || std::regex_search(seq::util::toStdString(clean), regex_number_2) ) return make( seq::Compiler::Token::Category::Number, 0 );
		if( raw == "<<"_b ) return make( seq::Compiler::Token::Category::Stream, 0 );
		if( clean == "{"_b || raw == "}"_b ) return make( seq::Compiler::Token::Category::FuncBracket, clean == "{"_b ? 1 : -1 );
		if( clean == "["_b || raw == "]"_b ) return make( seq::Compiler::Token::Category::FlowBracket, clean == "["_b ? 1 : -1 );
		if( clean == "("_b || raw == ")"_b ) return make( seq::Compiler::Token::Category::MathBracket, clean == "("_b ? 1 : -1 );
		if( raw == ","_b ) return make( seq::Compiler::Token::Category::Comma, 0 );
		if( raw == ":"_b ) return make( seq::Compiler::Token::Category::Colon, 0 );
		if( std::regex_search( seq::util::toStdString(clean), regex_arg) ) return make( seq::Compiler::Token::Category::Arg, clean.size() - 1 );

		if( clean.front() == '"'_b && clean.back() == '"'_b ) {
			clean = clean.substr(1, clean.size() - 2);
			return make( seq::Compiler::Token::Category::String, 0 );
		}

		auto it = std::find(operators.begin(), operators.end(), seq::util::toStdString(raw));
		if( it != operators.end() ) {
			return make( seq::Compiler::Token::Category::Operator, operatorDataFromString( raw, operator_weights.at(it - operators.begin()) ) );
		}
	}

	throw seq::CompilerError( seq::util::toStdString( "token: "_b + raw ), "", "", line );
}

int seq::Compiler::findStreamEnd( std::vector<seq::Compiler::Token>& tokens, int start, int end ) {
	int a = 0; // func_bracket counter
	int b = 0; // flow_bracket counter
	int c = 0; // math_bracket counter
	int l = 0; // current line

	try{
		l = tokens.at( start ).getLine();
	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "Invalid start index!" );
	}

	try{

		for( int index = start; index < end; index ++ ) {
			seq::Compiler::Token& token = tokens.at(index);

			if( (int) token.getLine() != l ) {

				if( a == 0 && b == 0 && c == 0 ) {
					return index - 1;
				}

				l = token.getLine();

			}

			if( index + 1 >= end ) {
				if( a == 0 && b == 0 && c == 0 ) {
					return index;
				}
			}

			if( token.getCategory() == seq::Compiler::Token::Category::FuncBracket ) a += ( token.getClean().at(0) == (byte) '}' ) ? -1 : 1;
			if( token.getCategory() == seq::Compiler::Token::Category::FlowBracket ) a += ( token.getClean().at(0) == (byte) ']' ) ? -1 : 1;
			if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) a += ( token.getClean().at(0) == (byte) ')' ) ? -1 : 1;

		}

	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "Invalid end index!" );
	}

	return ( a == 0 && b == 0 && c == 0 ) ? end - 1 : -1;
}

int seq::Compiler::findOpening( std::vector<seq::Compiler::Token>& tokens, int index, seq::Compiler::Token::Category type ) {
	index --;

	try {
		for( int i = -1; i != 0; index -- ) {
			auto& token = tokens.at(index);
			if( token.getCategory() == type ) i += token.getData();
		}
	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "No opening token found!" );
	}

	return index;
}

int seq::Compiler::findClosing( std::vector<seq::Compiler::Token>& tokens, int index, seq::Compiler::Token::Category type ) {
	index ++;

	try {
		for( int i = 1; i != 0; index ++ ) {
			auto& token = tokens.at(index);
			if( token.getCategory() == type ) i += token.getData();
		}
	}catch( std::out_of_range& err ) {
		throw seq::InternalError( "No closing token found!" );
	}

	return index;
}

std::vector<byte> seq::Compiler::assembleStream( std::vector<seq::Compiler::Token>& tokens, int start, int end, byte tags ) {

	enum struct State: byte {
		Start,
		Continue,
		Set,
		Function,
		Expression,
		Flowc,
		Stream
	};

	std::vector<byte> arr;
	seq::BufferWriter bw( arr );

	State state = State::Start;

	for( int i = start; i <= end; i ++ ) {

		auto& token = tokens.at(i);

		switch( state ) {

			case State::Start:
				if( token.getCategory() == seq::Compiler::Token::Category::Set ) {
					state = State::Set;
					break;
				}
				// no break //

			case State::Continue:
				if( token.getCategory() == seq::Compiler::Token::Category::Name ) {
					bw.putName( token.getAnchor(), false, token.getClean().c_str() );
					state = State::Stream;
					break;
				}

				if( token.isPrimitive() ) {
					std::vector<byte> arr1 = assemblePrimitive( token );
					arr.insert( arr.end(), arr1.begin(), arr1.end() );
					state = State::Stream;
					break;
				}

				if( token.getCategory() == seq::Compiler::Token::Category::FuncBracket ) {
					state = State::Function;
					break;
				}

				if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) {
					state = State::Expression;
					break;
				}

				if( token.getCategory() == seq::Compiler::Token::Category::FlowBracket ) {
					state = State::Flowc;
					break;
				}

				throw seq::CompilerError( seq::util::toStdString( "token '"_b + token.getRaw() + "'"_b ), "name, value, argument, function, expression or flow controller", "stream", token.getLine() );
				break;

			case State::Set:
				if( token.getCategory() == seq::Compiler::Token::Category::Name ) {
					if( !token.getAnchor() ) {
						bw.putName( token.getAnchor(), true, token.getClean().c_str() );
						state = State::Stream;
						break;
					}else{
						throw seq::CompilerError( "anchor after 'set' keyword", "name", "stream", token.getLine() );
					}
				}
				throw seq::CompilerError( seq::util::toStdString( "token: '"_b + token.getRaw() + "'"_b ), "name", "stream", token.getLine() );

			case State::Function: {
					int j = findClosing( tokens, i - 1, seq::Compiler::Token::Category::FuncBracket ) - 1;
					auto acc = assembleFunction( tokens, i, j, tokens.at(i - 1).getAnchor() );
					arr.insert( arr.end(), acc.begin(), acc.end() );
					i = j;
					state = State::Stream;
					break;
				}


			case State::Expression: {
					int j = findClosing( tokens, i - 1, seq::Compiler::Token::Category::MathBracket ) - 1;
					auto acc = assembleExpression( tokens, i, j, tokens.at(i - 1).getAnchor() );
					arr.insert( arr.end(), acc.begin(), acc.end() );
					i = j;
					state = State::Stream;
					break;
				}


			case State::Flowc: {
					int j = findClosing( tokens, i - 1, seq::Compiler::Token::Category::FlowBracket ) - 1;
					auto acc = assembleFlowc( tokens, i, j, tokens.at(i - 1).getAnchor() );
					arr.insert( arr.end(), acc.begin(), acc.end() );
					i = j;
					state = State::Stream;
					break;
				}


			case State::Stream:
				if( token.getCategory() != seq::Compiler::Token::Category::Stream ) {
					throw seq::CompilerError( seq::util::toStdString( "token '"_b + tokens.at(i).getRaw() + "'"_b ), "'<<'", "stream", token.getLine() );
				}else{
					state = State::Continue;
				}
				break;

		}

	}

	std::vector<byte> ret;
	seq::BufferWriter bw2( ret );
	bw2.putStream( false, tags, arr );

	return ret;

}

std::vector<byte> seq::Compiler::assemblePrimitive( seq::Compiler::Token token ) {

	std::vector<byte> arr;
	seq::BufferWriter bw( arr );
	const bool flag = token.getAnchor();

	try{

		switch( token.getCategory() ) {

			case seq::Compiler::Token::Category::Arg:
				bw.putArg(flag, token.getData() );
				break;

			case seq::Compiler::Token::Category::Null:
				bw.putNull(flag);
				break;

			case seq::Compiler::Token::Category::Bool:
				bw.putBool(flag, (bool) token.getData());
				break;

			case seq::Compiler::Token::Category::Number:
				bw.putNumber(flag, seq::util::asFraction( std::stod( seq::util::toStdString( token.getClean() ) ) ) );
				break;

			case seq::Compiler::Token::Category::Type:
				bw.putType(flag, seq::util::toDataType( token.getClean() ));
				break;

			case seq::Compiler::Token::Category::String:
				bw.putString(flag, token.getClean().c_str());
				break;

			case seq::Compiler::Token::Category::VMC: {
				bw.putCall(flag, (seq::type::VMCall::CallType) token.getData());
				break;
			}


			default:
				throw seq::InternalError( "Invalid argument " + token.toString() + "!" );
		}

	}catch( ... ){
		throw seq::InternalError( "Invalid argument " + token.toString() + "!" );
	}

	return arr;
}

std::vector<byte> seq::Compiler::assembleFlowc( std::vector<seq::Compiler::Token>& tokens, int start, int end, bool anchor ) {

	std::vector<std::vector<byte>> arr;

	bool expectSeparator = false;
	int i = start;

	for( ; i < end; i ++ ) {

		seq::Compiler::Token& token = tokens.at( i );

		if( token.getAnchor() ) {
			throw seq::CompilerError( "anchor", "", "flow controller", token.getLine() );
		}

		if( expectSeparator ) {

			if( token.getCategory() == seq::Compiler::Token::Category::Comma ) {
				expectSeparator = false;
			}else{
				throw seq::CompilerError( "token '" + seq::util::toStdString(tokens.at( i ).getRaw()) + "'", "','", "flow controller", tokens.at(i).getLine() );
			}

		}else{

			switch( token.getCategory() ) {

				case seq::Compiler::Token::Category::Number: {

					if( i + 1 < end && tokens.at( i + 1 ).getCategory() == seq::Compiler::Token::Category::Colon ) {

						if( i + 2 < end && tokens.at( i + 2 ).getCategory() == seq::Compiler::Token::Category::Number ) {

							if( tokens.at( i + 2 ).getAnchor() ) {
								throw seq::CompilerError( "anchor", "", "flow controller", token.getLine() );
							}else{
								std::vector<byte> arr1 = assemblePrimitive( token );
								std::vector<byte> arr2 = assemblePrimitive( tokens.at( i + 2 ) );
								arr1.reserve( arr1.size() + arr2.size() );
								arr1.insert( arr1.end(), arr2.begin(), arr2.end() );
								arr.push_back( arr1 );

								i += 2;
								expectSeparator = true;
							}

						}else{
							throw seq::CompilerError( "token '" + seq::util::toStdString(tokens.at( i + 2 ).getRaw()) + "'", "number", "flow controller", tokens.at(i + 2).getLine() );
						}

					}else{
						arr.push_back( assemblePrimitive( token ) );
						expectSeparator = true;
					}

					break;
				}

				case seq::Compiler::Token::Category::String:
				case seq::Compiler::Token::Category::Bool:
				case seq::Compiler::Token::Category::Type:
				case seq::Compiler::Token::Category::Null: {
					arr.push_back( assemblePrimitive( token ) );
					expectSeparator = true;
					break;
				}

				default:
					throw seq::CompilerError( "token '" + seq::util::toStdString(tokens.at( i ).getRaw()) + "'", "value or rage", "flow controller", tokens.at(i).getLine() );

			}

		}

	}

	if( !expectSeparator || arr.empty() ) {
		throw seq::CompilerError( "", "value or rage", "flow controller", tokens.at(i).getLine() );
	}

	std::vector<byte> ret;
	seq::BufferWriter bw( ret );
	bw.putFlowc(anchor, arr);

	return ret;

}

std::vector<byte> seq::Compiler::assembleExpression( std::vector<seq::Compiler::Token>& tokens, int start, int end, bool anchor ) {

	if( end - start == 1 ) {
		return assemblePrimitive( tokens.at( start ) );
	}

	int h = -1;
	int j = -1;
	int l = 0;
	int f = 0;
	bool eop = false;

	for( int i = start; i < end; i ++ ) {

		auto& token = tokens.at( i );

		if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) {

			if( token.getData() == 1 ) {
				if( eop ) throw seq::CompilerError( "", "operator", "expression", token.getLine() );
			}else{
				if( !eop ) throw seq::CompilerError( "operator", "", "expression", token.getLine() );
			}

		} else {
			if( eop && token.getCategory() != seq::Compiler::Token::Category::Operator ) {
				throw seq::CompilerError( "", "operator", "expression", token.getLine() );
			}else if( !eop && token.getCategory() == seq::Compiler::Token::Category::Operator ) {
				throw seq::CompilerError( "operator", "", "expression", token.getLine() );
			}
			eop = !eop;
		}

	}

	while ( true ) {

		for( int i = start; i < end; i ++ ) {

			auto& token = tokens.at( i );

			if( token.getCategory() == seq::Compiler::Token::Category::MathBracket ) {
				l += token.getData();
			}

			if( l == f && token.getCategory() == seq::Compiler::Token::Category::Operator ) {
				int tmp = token.getData() & 0b11111111;
				if( h < tmp ) {
					h = tmp;
					j = i;
				}
			}

		}

		if( h == -1 ) {
			if( f != 0 ) {
				throw seq::CompilerError( "end of expression", "operator", "expression", tokens.at(end - 1).getLine() );
			}

			l = 0;
			f = 1;
		}else{
			break;
		}

	}

	auto lt = assembleExpression( tokens, start + f + 0, j, false );
	auto rt = assembleExpression( tokens, j + 1, end - f, false );
	seq::ExprOperator op = (seq::ExprOperator) (tokens.at(j).getData() >> 8);

	std::vector<byte> arr;
	seq::BufferWriter bw( arr );
	bw.putExpr(anchor, op, lt, rt);

	return arr;
}

std::vector<byte> seq::Compiler::assembleFunction( std::vector<seq::Compiler::Token>& tokens, int start, int end, bool anchor ) {

	std::vector<byte> arr;

	if( end - start < 2 ) {
		throw seq::CompilerError( "end of scope", "stream", "function", tokens.at(start).getLine() );
	}

	for( int i = start; i < end; i ++ ) {

		byte tags = 0;

		if( tokens.at(i).getCategory() == seq::Compiler::Token::Category::Tag ) {
			tags = tokens.at(i).getData();
			if( (++ i) >= end ) {
				throw seq::CompilerError( "end of input", "start of stream", "function", tokens.at(i).getLine() );
			}
		}

		int j = findStreamEnd( tokens, i, end );
		if( j != -1 ) {

			auto stream = assembleStream( tokens, i, j, tags );
			arr.insert( arr.end(), stream.begin(), stream.end() );
			i = j;

		}else{
			throw seq::CompilerError( "end of input", "end of stream", "function", tokens.at(i).getLine() );
		}

	}

	std::vector<byte> ret;
	seq::BufferWriter bw( ret );
	bw.putFunc(anchor, arr);

	return ret;
}

int seq::Compiler::extractHeaderData( std::vector<Token>& tokens, std::vector<seq::string>* arrayPtr ) {

	int s = 0;

	if( tokens.size() > 0 && tokens.at(0).getCategory() == seq::Compiler::Token::Category::Load ) {
		int l = 0;
		const int size = tokens.size();

		for( int i = 0; i < size; i ++ ) {
			auto& token = tokens.at(i);

			if( l != (int) token.getLine() ) {

				l = token.getLine();

				if( token.getCategory() == seq::Compiler::Token::Category::Load ) {

					if( i + 1 >= size || (i + 2 >= size && (int) tokens.at( i + 2 ).getLine() == l) ) {
						throw seq::CompilerError( "token " + seq::util::toStdString( token.getRaw() ), "load statement", "header", l );
					}

					auto& token2 = tokens.at( i + 1 );

					if( token2.getCategory() != seq::Compiler::Token::Category::String || token2.getAnchor() ) {
						throw seq::CompilerError( "token " + seq::util::toStdString( token.getRaw() ), "load statement", "header", l );
					}

					if( arrayPtr != nullptr ) {
						arrayPtr->push_back( token2.getClean() );
					}

					s = i;
					continue;
				}

				s = i;
				break;

			}
		}
	}

	return s;

}

#endif // SEQ_EXCLUDE_COMPILER

#undef SEQ_IMPLEMENT
#endif // SEQ_IMPLEMENT
