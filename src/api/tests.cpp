
#include "SeqAPI.hpp"
#include "../lib/vstl.hpp"

// Test coverage: 91.89%
// Last updated: 2020-11-26

using seq::byte;

void print_buffer( seq::ByteBuffer& bb ) {
    seq::BufferReader br = bb.getReader();
    while( br.hasNext() ) std::cout << (int) br.nextByte() << " ";
}

seq::Stream native_join_strings( seq::Stream& input ) {
	std::string str;

	for( auto& arg : input ) {
		if( arg.getDataType() == seq::DataType::String ) {
			str += arg.String().getString();
		}else{
			throw seq::RuntimeError( "Invalid argument for 'join', string expected!" );
		}
	}
	input.clear();

	return { seq::Generic( new seq::type::String( false, str.c_str() ) ) };
}


TEST( buffer_reader_simple, {

    byte buffer[] = { 'A', 'B', 'C', 'D' };
    seq::ByteBuffer bb = seq::ByteBuffer( buffer, 4 );
    seq::BufferReader br = bb.getReader();

    CHECK( br.nextByte(), (byte) 'A' );
    CHECK( br.peekByte(), (byte) 'B' );
    CHECK( br.nextByte(), (byte) 'B' );
} );


TEST( buffer_writer_simple, {

    std::vector<byte> buf;
    seq::BufferWriter bw( buf );

    bw.putByte( 'S' );
    bw.putBool( true, true );
    bw.putBool( true, false );
    bw.putBool( false, true );
    bw.putBool( false, false );
    bw.putNull( false );

    CHECK( buf[0], (byte) 'S' );
    CHECK( buf[1], (byte) ((byte) seq::Opcode::BLT | 0b10000000) );
    CHECK( buf[2], (byte) ((byte) seq::Opcode::BLF | 0b10000000) );
    CHECK( buf[3], (byte) ((byte) seq::Opcode::BLT | 0b00000000) );
    CHECK( buf[4], (byte) ((byte) seq::Opcode::BLF | 0b00000000) );
    CHECK( buf[5], (byte) ((byte) seq::Opcode::NIL | 0b00000000) );

} );


TEST( buffer_writer_complex, {

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    bw.putByte( 'S' );
    bw.putBool( true, true );
	seq::Fraction frac1{ 12, 1 };
	seq::Fraction frac2{ 1, 2 };
	seq::Fraction frac3{ 1422131241, 1 };
    bw.putNumber( false,  frac1 );
    bw.putNumber( true, frac2 );
    bw.putNumber( false, frac3 );
    bw.putString( true, "Hello World!" );
    bw.putType( false, seq::DataType::Number );
    bw.putCall( false, seq::type::VMCall::CallType::Return );
    bw.putArg( false, 3 );
    bw.putName( false, true, "name" );

    { // fraction (special case - default values for size 0)
    	bw.putOpcode( true, seq::Opcode::NUM );
    	bw.putHead( 0, 0 );
    }

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    CHECK( br.nextByte(), (byte) 'S' );

    { // bool
    	seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Bool );
        CHECK( tr.isAnchored(), true );
        CHECK( tr.getGeneric().Bool().getBool(), true );
    }

    { // small number
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
        CHECK( tr.isAnchored(), false );
        CHECK( tr.getGeneric().Number().getLong(), (long) 12 );
    }

    { // fraction
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
        CHECK( tr.isAnchored(), true );
        CHECK( tr.getGeneric().Number().getDouble(), (double) 0.5 );
    }

    { // big number
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
        CHECK( tr.isAnchored(), false );
        CHECK( tr.getGeneric().Number().getLong(), (long) 1422131241 );
    }

    { // string
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::String );
        CHECK( tr.isAnchored(), true );
        CHECK_ELSE( tr.getGeneric().String().getString(), std::string( "Hello World!" ) ) {
        	FAIL( "Invalid string!" );
        }
    }

    { // type
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Type );
        CHECK( tr.isAnchored(), false );
        CHECK( (byte) tr.getGeneric().Type().getType(), (byte) seq::DataType::Number );
    }

    { // VM call
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::VMCall );
        CHECK( tr.isAnchored(), false );
        CHECK( (byte) tr.getGeneric().VMCall().getCall(), (byte) seq::type::VMCall::CallType::Return );
    }

    { // arg
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Arg );
        CHECK( tr.isAnchored(), false );
        CHECK( tr.getGeneric().Arg().getLevel(), (byte) 3 );
    }

    { // name (variable)
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Name );
        CHECK( tr.isAnchored(), false );
        seq::Generic generic = tr.getGeneric();
        auto& name = generic.Name();
        CHECK( name.getDefine(), true );
        CHECK_ELSE( name.getName(), std::string( "name" ) ) {
            FAIL( "Invalid string! - " + std::string( (char*) name.getName().c_str() ) );
        }
    }

    { // fraction (special case)
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Number );
        CHECK( tr.isAnchored(), true );
        CHECK( tr.getGeneric().Number().getLong(), (long) 0 );
    }

} );


TEST( buffer_writer_function, {

    std::vector<byte> func_arr;
    seq::BufferWriter func_bw( func_arr );
    func_bw.putByte( (byte) 'A' );
    func_bw.putByte( (byte) 'B' );
    func_bw.putByte( (byte) 'C' );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );
    bw.putFunc( false, func_arr, true );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    seq::TokenReader tr = br.next();
    CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Func );
    CHECK( tr.isAnchored(), false );

    {
    	auto g = tr.getGeneric();
        seq::BufferReader func_br = tr.getGeneric().Function().getReader();
        CHECK( func_br.nextByte(), (byte) 'A' );
        CHECK( func_br.nextByte(), (byte) 'B' );
        CHECK( func_br.nextByte(), (byte) 'C' );
        CHECK( func_br.nextByte(), (byte) 0 );
    }

    {
    	auto g = tr.getGeneric();
        seq::BufferReader func_br = g.Function().getReader();
        CHECK( func_br.nextByte(), (byte) 'A' );
        CHECK( func_br.nextByte(), (byte) 'B' );
        CHECK( func_br.nextByte(), (byte) 'C' );
        CHECK( func_br.nextByte(), (byte) 0 );
    }

    seq::TokenReader tr2 = br.next();
    CHECK( (byte) tr2.getDataType(), (byte) seq::DataType::Null );
    CHECK( tr2.isAnchored(), false );
} );


TEST( buffer_writer_expression, {

    std::vector<byte> left_arr;
    seq::BufferWriter left_bw( left_arr );
    left_bw.putByte( (byte) 'A' );
    left_bw.putByte( (byte) 'B' );
    left_bw.putByte( (byte) 'C' );

    std::vector<byte> right_arr;
    seq::BufferWriter right_bw( right_arr );
    right_bw.putByte( (byte) '1' );
    right_bw.putByte( (byte) '2' );
    right_bw.putByte( (byte) '3' );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );
    bw.putExpr( false, seq::ExprOperator::Addition, left_arr, right_arr );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    seq::TokenReader tr = br.next();
    CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Expr );
    CHECK( tr.isAnchored(), false );

    CHECK( (byte) tr.getGeneric().Expression().getOperator(), (byte) seq::ExprOperator::Addition );

    {
    	seq::Generic generic = tr.getGeneric();
        seq::BufferReader expr_br = generic.Expression().getLeftReader();
        CHECK( expr_br.nextByte(), (byte) 'A' );
        CHECK( expr_br.nextByte(), (byte) 'B' );
        CHECK( expr_br.nextByte(), (byte) 'C' );
        CHECK( expr_br.nextByte(), (byte) 0 );
    }

    {
    	seq::Generic generic = tr.getGeneric();
        seq::BufferReader expr_br = generic.Expression().getRightReader();
        CHECK( expr_br.nextByte(), (byte) '1' );
        CHECK( expr_br.nextByte(), (byte) '2' );
        CHECK( expr_br.nextByte(), (byte) '3' );
        CHECK( expr_br.nextByte(), (byte) 0 );
    }

    seq::TokenReader tr2 = br.next();
    CHECK( (byte) tr2.getDataType(), (byte) seq::DataType::Null );
    CHECK( tr2.isAnchored(), false );

} );


TEST( buffer_writer_flowc, {

    std::vector<byte> a_arr;
    seq::BufferWriter a_bw( a_arr );
	seq::Fraction fraction1 { 12, 1 };
	seq::Fraction fraction2 { 24, 1 };
    a_bw.putNumber( false, fraction1 );
    a_bw.putNumber( false, fraction2 );

    std::vector<byte> b_arr;
    seq::BufferWriter b_bw( b_arr );
    b_bw.putBool( false, false );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    std::vector<std::vector<byte>> bvv = { a_arr, b_arr };
    bw.putFlowc( false, bvv );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    {
        seq::TokenReader tr = br.next();
        CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Flowc );
        CHECK( tr.isAnchored(), false );

        seq::Generic generic = tr.getGeneric();
        std::vector< seq::FlowCondition* > fcs = generic.Flowc().getConditions();

        CHECK( fcs.size(), (size_t) 2 );
        CHECK( (byte) fcs.at(0)->type, (byte) seq::FlowCondition::Type::Range );
        CHECK( fcs.at(0)->a.Number().getLong(), (long) 12);
        CHECK( fcs.at(0)->b.Number().getLong(), (long) 24);
        CHECK( (byte) fcs.at(1)->type, (byte) seq::FlowCondition::Type::Value );
        CHECK( fcs.at(1)->a.Bool().getBool(), false );
    }

} );


TEST( buffer_writer_stream, {

    std::vector<byte> stream_arr;
    seq::BufferWriter stream_bw( stream_arr );
    stream_bw.putByte( (byte) 'A' );
    stream_bw.putByte( (byte) 'B' );
    stream_bw.putByte( (byte) 'C' );

    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    bw.putStream( true, 0b00000011, stream_arr );
    bw.putNull( false );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();

    seq::TokenReader tr = br.next();
    CHECK( (byte) tr.getDataType(), (byte) seq::DataType::Stream );
    CHECK( tr.isAnchored(), true );

    auto g = tr.getGeneric();
    seq::BufferReader sbr = g.Stream().getReader();
    CHECK( sbr.nextByte(), (byte) 'A' );
    CHECK( sbr.nextByte(), (byte) 'B' );
    CHECK( sbr.nextByte(), (byte) 'C' );
    CHECK( sbr.nextByte(), (byte) 0 );

    seq::TokenReader tr2 = br.next();
    CHECK( (byte) tr2.getDataType(), (byte) seq::DataType::Null );
    CHECK( tr2.isAnchored(), false );


} );


TEST( buffer_writer_file_header, {
    std::vector<byte> arr;
    seq::BufferWriter bw( arr );

    std::map<std::string, std::string> data;
    data[ "test"] = "Hello World!";
    data[ "Foo"] = "Bar";
    data[ "number"] = "123.4";

    bw.putFileHeader( 1, 2, 3, data );
    bw.putByte( (byte) 'A' );

    seq::ByteBuffer bb( arr.data(), arr.size() );
    seq::BufferReader br = bb.getReader();
    seq::FileHeader header = br.getHeader();

    CHECK( header.checkVersion( 1, 2 ), true );
    CHECK( header.checkVersion( 1, 3 ), false );
    CHECK( header.checkPatch( 3 ), true );
    CHECK( header.checkPatch( 4 ), false );

    CHECK_ELSE( header.getValue( "test" ), std::string( "Hello World!" ) ) {
        FAIL( "Invalid string! - " + std::string( (char*) header.getValue( "test" ).c_str() ) );
    }

    CHECK_ELSE( header.getValue( "Foo" ), std::string( "Bar" ) ) {
        FAIL( "Invalid string! - " + std::string( (char*) header.getValue( "Foo" ).c_str() ) );
    }

    CHECK_ELSE( header.getValue( "number" ), std::string( "123.4" ) ) {
        FAIL( "Invalid string! - " + std::string( (char*) header.getValue( "number" ).c_str() ) );
    }

    CHECK( header.getVersionMajor(), 1 );
    CHECK( header.getVersionMinor(), 2 );
    CHECK( header.getVersionPatch(), 3 );

    seq::FileHeader header2( header );

    CHECK( header2.getVersionMajor(), 1 );
    CHECK( header2.getVersionMinor(), 2 );
    CHECK( header2.getVersionPatch(), 3 );

    seq::FileHeader header3;

    CHECK_ELSE( header3.getVersionString(), std::string( "0.0.0" ) ) {
    	FAIL( "Invalid version string, expected '0.0.0'!" );
    }

    seq::FileHeader header4( std::move( header2 ) );

    CHECK_ELSE( header4.getVersionString(), std::string( "1.2.3" ) ) {
    	FAIL( "Invalid version string, expected '1.2.3'!" );
    }

    CHECK( header4.getValueMap().empty(), false );
    CHECK( header2.getValueMap().empty(), true );
    CHECK( header3.getValueMap().empty(), true );
    CHECK( header.getValueMap().empty(), false );

    header4 = header3;

    CHECK_ELSE( header4.getVersionString(), std::string( "0.0.0" ) ) {
    	FAIL( "Invalid version string, expected '0.0.0'!" );
    }

    CHECK( header4.getValueMap().empty(), true );

    CHECK( br.nextByte(), (byte) 'A' );
} );


TEST( executor_hello_world, {

		std::vector<byte> arr_1;
		seq::BufferWriter bw_1( arr_1 );
		bw_1.putCall( true, seq::type::VMCall::CallType::Exit );
		bw_1.putString( false, "Hello World!" );

		std::vector<byte> arr_2;
		seq::BufferWriter bw_2( arr_2 );
		bw_2.putStream( false, 0, arr_1 );

		seq::ByteBuffer bb( arr_2.data(), arr_2.size() );

		seq::Executor exe;
		exe.execute( bb );

		CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
		CHECK_ELSE( exe.getResult().String().getString(), std::string( "Hello World!" ) ) {
			FAIL( "Invalid String!" );
		}

} );


TEST( executor_hello_world_var, {

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	bw_1.putName( false, true, "var" );
	bw_1.putString( false, "Hello World!" );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	bw_2.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_2.putName( false, false, "var" );

	std::vector<byte> arr_3;
	seq::BufferWriter bw_3( arr_3 );
	bw_3.putStream( false, 0, arr_1 );
	bw_3.putStream( false, 0, arr_2 );

	seq::ByteBuffer bb( arr_3.data(), arr_3.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), std::string( "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( executor_hello_world_func, {

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	bw_1.putCall( true, seq::type::VMCall::CallType::Return );
	bw_1.putString( false, "Hello World!" );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	bw_2.putStream( false, 0, arr_1 );

	std::vector<byte> arr_3;
	seq::BufferWriter bw_3( arr_3 );
	bw_3.putName( false, true, "var" );
	bw_3.putFunc( false, arr_2, true );

	std::vector<byte> arr_4;
	seq::BufferWriter bw_4( arr_4 );
	bw_4.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_4.putName( true, false, "var" );
	bw_4.putNull( false );

	std::vector<byte> arr_5;
	seq::BufferWriter bw_5( arr_5 );
	bw_5.putStream( false, 0, arr_3 );
	bw_5.putStream( false, 0, arr_4 );

	seq::ByteBuffer bb( arr_5.data(), arr_5.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), std::string( "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( executor_native, {

	seq::Executor exe;
	exe.inject( "sum", [] ( seq::Stream& input ) -> seq::Stream {
		double sum = 0;
		for( auto& arg : input ) {
			sum += seq::util::numberCast( arg ).Number().getDouble();
		}
		input.clear();

		seq::Stream acc;
		acc.push_back( seq::Generic( new seq::type::Number( false, sum ) ) );
		return acc;
	} );

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	bw_1.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_1.putName( true, false, "sum" );
	seq::Fraction fract1{ 1, 1 };
	seq::Fraction fract2{ 2, 1 };
	seq::Fraction fract3{ 3, 1 };
	seq::Fraction fract4{ 4, 1 };
	bw_1.putNumber( false, fract1 );
	bw_1.putNumber( false, fract2 );
	bw_1.putNumber( false, fract3 );
	bw_1.putNumber( false, fract4 );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	bw_2.putStream( false, 0, arr_1 );

	seq::ByteBuffer bb( arr_2.data(), arr_2.size() );

	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 10 );

} );

TEST( expression_simple, {

	std::vector<byte> arr_1;
	seq::BufferWriter bw_1( arr_1 );
	seq::Fraction fract1{ 6, 1 };
	bw_1.putNumber( false, fract1 );

	std::vector<byte> arr_2;
	seq::BufferWriter bw_2( arr_2 );
	seq::Fraction fract2{ 7, 1 };
	bw_2.putNumber( false, fract2 );

	std::vector<byte> arr_3;
	seq::BufferWriter bw_3( arr_3 );
	bw_3.putCall( true, seq::type::VMCall::CallType::Exit );
	bw_3.putExpr( false, seq::ExprOperator::Multiplication, arr_1, arr_2 );

	std::vector<byte> arr_4;
	seq::BufferWriter bw_4( arr_4 );
	bw_4.putStream( false, 0, arr_3 );

	seq::ByteBuffer bb( arr_4.data(), arr_4.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 42 );

} );

TEST( tokenizer_basic, {

	std::vector<seq::Compiler::Token> tokens = seq::Compiler::tokenize( "#exit << 1 << \"Hello\" << 3" );

	if( tokens.at(0).getCategory() != seq::Compiler::Token::Category::VMCall ) {
		FAIL( "Expected 'VMCall' token at index 0, found " + tokens.at(0).toString() );
	}

	if( tokens.at(1).getCategory() != seq::Compiler::Token::Category::Stream ) {
		FAIL( "Expected 'Stream' token at index 1, found " + tokens.at(1).toString() );
	}

	if( tokens.at(2).getCategory() != seq::Compiler::Token::Category::Number ) {
		FAIL( "Expected 'Number' token at index 2, found " + tokens.at(2).toString() );
	}

	if( tokens.at(6).getCategory() != seq::Compiler::Token::Category::Number ) {
		FAIL( "Expected 'Number' token at index 6, found " + tokens.at(6).toString() );
	}

} );

TEST( ce_hello_world, {

	std::string code = "#exit << \"Hello World!\"";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), std::string( "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_hello_world_func, {

	std::string code = (
			"set var << {\n"
			"	#return << \"Hello World!\"\n"
			"}\n"
			"#exit << #var << null\n"
			);

	auto tokens = seq::Compiler::tokenize( code );

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), std::string( "Hello World!" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_expression, {

	std::string code = "#exit << ( 1 + 2 + 3 + 4 )";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 10 );

} );


TEST( ce_expression_nested_right, {

	std::string code = "#exit << ( 1 + ( 2 + 3 ) )";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 6 );

} );

TEST( ce_expression_nested_left, {

	std::string code = "#exit << ( ( 2 + 3 ) + 1 )";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 6 );

} );


TEST( ce_expression_nested_double, {

	std::string code = "#exit << ( ( 2 + 3 ) + ( 4 + 1 ) )";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), (long) 10 );

} );

TEST( ce_expression_float, {

	std::string code = "#exit << ( ( 2.14 + 3.41 ) * 100 )";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 555l );

} );


TEST( ce_expression_complex, {

	std::string code = (
			"set var << {\n"
			"	#return << @\n"
			"}\n"
			"#exit << #var << 10\n");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 10l );

} );

TEST( ce_arg, {

	std::string code = (
			"#{\n"
			"	#exit << #{\n"
			"		#return << (@ + @@) \n"
			"	} << 20\n"
			"} << 10\n");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 30l );

} );


TEST( ce_string_escape_codes, {

	std::string code = (
			"#exit << \"\\\\\\n\\t\\\"\\r\"");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), std::string( "\\\n\t\"\r" ) ) {
		FAIL( "Invalid String!" );
	}

} );


TEST( ce_flowc_1, {

	std::string code = (
			"#exit << #[true] << true");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Bool );
	CHECK( exe.getResult().Bool().getBool(), true );

} );

TEST( ce_flowc_2, {

	std::string code = (
			"#exit << #[true] << false << true");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Bool );
	CHECK( exe.getResult().Bool().getBool(), true );

} );

TEST( ce_flowc_3, {

	std::string code = (
			"#exit << #[number] << true << null << \"Hello\" << 2");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 2l );

} );

TEST( ce_flowc_4, {

	std::string code = (
			"#exit << #[2] << true << null << \"Hello\" << 2");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 2l );

} );


TEST( ce_type_cast_bool_1, {

	std::string code = ("#exit << #bool << 1 << null << \"hello\"");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(0).Bool().getBool(), true );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(1).Bool().getBool(), false );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(2).Bool().getBool(), false );

} );

TEST( ce_type_cast_bool_2, {

	std::string code = ("#exit << #bool << 0 << 2 << 0.00234");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(0).Bool().getBool(), false );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(1).Bool().getBool(), true );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(2).Bool().getBool(), true );

} );

TEST( ce_type_cast_number_1, {

	std::string code = ("#exit << #number << \"123\" << \"42.5\" << break");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 123l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getDouble(), 42.5 );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 1l );

} );

TEST( ce_type_cast_number_2, {

	std::string code = ("#exit << #number << true << 6 << null");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 6l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 0l );

} );

TEST( ce_type_cast_string_1, {

	std::string code = ("#exit << #string << true << null << 69");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(0).String().getString(), std::string( "true" ) ) {
		FAIL( "Invalid String!" );
	}

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(1).String().getString(), std::string( "null" ) ) {
		FAIL( "Invalid String!" );
	}

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(2).String().getString(), std::string( "69" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_type_cast_string_2, {

	std::string code = ("#exit << #string << 123.2 << 42 << \"hello\" ");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::String );
	CHECK( std::stod( res.at(0).String().getString() ), 123.2 );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::String );
	CHECK( std::stod( res.at(1).String().getString() ), 42.0 );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(2).String().getString(), std::string( "hello" ) ) {
		FAIL( "Invalid String!" );
	}

} );


TEST( ce_stream_tags, {

	std::string code = (
			"#exit << #join << #{\n"
            "	first; #return << \"first\"\n"
            "	last; #return << \"last\"\n"
            "	#return << \"none\"\n"
            "	end; #return << \"end\"\n"
			"} << 1 << 2 << 3");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.inject( "join", native_join_strings );
	exe.execute( bb );
	exe.reset();

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), std::string( "firstnonenonelastnoneend" ) ) {
		FAIL( "Invalid String!" );
	}

} );

TEST( ce_complex_sum, {

	std::string code = (
			"#exit << #{\n"
			"	first; set x << 0\n"
			"	set x << #{\n"
			"		#return << (@@ + @)\n"
			"	} << x\n"
			"	last; #return << x\n"
			"} << 1 << 2 << 3 << 4\n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 10l );

} );

TEST( ce_header, {

	std::string code = (
			"load \"test-1\" \n"
			"load \"test-2\" \n"
			"#exit << 1 \n"
			);

	std::vector<std::string> loades;

	auto buf = seq::Compiler::compile( code, &loades );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 1l );
	CHECK( (long) loades.size(), 2l );

	CHECK_ELSE( loades.at(0), std::string( "test-1" ) ) {
		FAIL( "Invalid String!" );
	}

	CHECK_ELSE( loades.at(1), std::string( "test-2" ) ) {
		FAIL( "Invalid String!" );
	}

} );


TEST( ce_simple_loop, {

	// TODO investigate why this is so slow (in valgrind)

	std::string code = (
			"#exit << #join << #string << #{\n"
            "	#return << @\n"
            "	#again << #( @ - 1 ) << #[true] << ( @ > 0 )\n"
            "} << 10\n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.inject( "join", native_join_strings );
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( exe.getResult().String().getString(), std::string( "109876543210" ) ) {
		std::cout << exe.getResult().String().getString();
		FAIL( "Invalid String!" );
	}

} );


TEST( ce_fibonacci_recursion, {

	std::string code = (
			"set sum << { \n"
			"	first; set x << 0 \n"
			"	set x << #{ \n"
			"		#return << (@@ + @) \n"
			"	} << x \n"
			"	last; #return << x \n"
			"} \n"
			" \n"
			"set fib << { \n"
			"	#final << #@ << #[true] << (@ <= 1) \n"
			"	#return << #sum << #fib << (@ - 1) << (@ - 2) \n"
			"} \n"
			" \n"
			"#exit << #fib << 9 << 11 << 6 << 12 \n"
	);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 34l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 89l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 8l );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(3).Number().getLong(), 144l );

} );


TEST( ce_prime_numbers, {

	std::string code = (
			"set isPrime << { \n"
			"	#return << #{ \n"
			"		#final << #false << #[true] << (@@ % @ = 0) \n"
			"		#again << #(@ - 1) << #[true] << (@ > 2) \n"
			"		end; #return << true"
			"	} << (@ - 1) \n"
			"} \n"
			" \n"
			"#exit << #isPrime << 7 << 11 << 6 << 13 << 64 << 4 \n"
	);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(0).Bool().getBool(), true );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(1).Bool().getBool(), true );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(2).Bool().getBool(), false );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(3).Bool().getBool(), true );

	CHECK( (byte) res.at(4).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(4).Bool().getBool(), false );

	CHECK( (byte) res.at(5).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(5).Bool().getBool(), false );

} );


TEST( ce_factorial_recursion, {

	std::string code = (
			"set factorial << { \n"
			"	#return << #{"
			"		#final << #1 << #[true] << (@ <= 0) \n"
			"		#return << #{ \n"
			"			#final << (@@ * @) \n"
			"		} << #factorial << (@ - 1) \n"
			"	} << @ \n"
			"} \n"
			" \n"
			"#exit << #factorial << 1 << 5 << 7 << 3 \n"
	);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 120l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 5040l );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(3).Number().getLong(), 6l );

} );


TEST( ce_order_1, {

	std::string code = ("#exit << 1 << 2 << 3");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 2l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 3l );

} );

TEST( ce_order_2, {

	std::string code = "#exit << #{ #return << @ } << 1 << 2 << 3";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 2l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 3l );

} );


TEST( ce_order_3, {

	std::string code = "#exit << #[number] << 1 << 2 << 3";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 2l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 3l );

} );

TEST( ce_order_4, {

	std::string code = "#exit << #number << 1 << 2 << 3";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 2l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 3l );

} );

TEST( ce_order_5, {

	std::string code = (
			"set var << 1 << 2 << 3 \n"
			"#exit << var"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 2l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 3l );

} );

TEST( ce_expr_complex_math, {

	std::string code = (
			"#exit << (\n"
			"	8 ** 2 * 9 - 5 * (( 12 + 12 - 25 ) ** 2) / 5 \n"
			")"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 575l );

} );

TEST( ce_expr_complex_logic, {

	// NEW MEMEORY PROBLEM

	std::string code = (
			"#exit << (\n"
			"(true || false) && ( true ^^ false ) && !false && !( 1 > 10 )\n"
			")"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Bool );
	CHECK( exe.getResult().Bool().getBool(), true );

} );

TEST( ce_deep_function, {

	std::string code = (
			"#exit << #{ #return << #{ #return << #{ #return << (@ * 2) } << (@ + 1) } << (@**2) } << 2"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 10l );

} );

TEST( ce_top_args, {

	std::string code = (
			"#exit << @"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Stream args = {
			seq::Generic( new seq::type::Number( false, 2 ) )
	};

	seq::Executor exe;
	exe.execute( bb, args );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 2l );

} );

TEST( ce_namespace, {

	std::string code = (
		"set foo:bar << 123 \n"
		"#exit << foo:bar"
	);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Number );
	CHECK( exe.getResult().Number().getLong(), 123l );

} );

TEST( ce_blob, {

	// create blob
	class Thing: public seq::type::Blob {

		public:
			Thing( bool _anchor, int a, int b ): seq::type::Blob( _anchor ) {
				this->a = a;
				this->b = b;
			}

			seq::type::Blob* copy() {
				return new Thing( this->anchor, this->a, this->b );
			}

			std::string toString() {
				return "blobus";
			}

			int a;
			int b;

	};

	std::string code = (
		"#exit << #unpack << #pack << null"
	);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.inject( "pack", [] ( seq::Stream& input ) -> seq::Stream {
		return seq::Stream {
			seq::Generic( new Thing( false, 123, 456 ) )
		};
	} );

	exe.inject( "unpack", [] ( seq::Stream& input ) -> seq::Stream {
		if( input[0].getDataType() != seq::DataType::Blob ) {
			return seq::Stream();
		}else{

			if( input[0].Blob().toString() != "blobus" ) {
				FAIL( "Invalid blob string!" );
			}

			return seq::Stream {
				seq::Generic( new seq::type::Number( false, ((Thing&) input[0].Blob()).a ) ),
				seq::Generic( new seq::type::Number( false, ((Thing&) input[0].Blob()).b ) )
			};
		}
	} );

	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 123l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 456l );

} )

TEST( ce_accessor_operator, {

	std::string code = (
			"set var << 123 << 456 << 789 \n"
			"#exit << ( var :: 0 ) << ( var :: 1 ) << ( var :: 2 ) << ( var :: 3 )"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 123l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 456l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 789l );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Null );

} );

TEST( c_fail_expression_anchor, {

	std::string code = (
			"#exit << ( #0 < 1 )"
			);

	try{
		seq::Compiler::compile( code );
	}catch( seq::CompilerError& err ) {
		return;
	}

	FAIL( "Expected exception!" )

} );

TEST( c_fail_empty_identifier, {

	std::string code = (
			"#exit << #"
			);

	try{
		seq::Compiler::compile( code );
	}catch( seq::CompilerError& err ) {
		return;
	}

	FAIL( "Expected exception!" )

} );

TEST( c_fail_stream_ending, {

	std::string code = (
			"#exit <<"
			);

	try{
		seq::Compiler::compile( code );
	}catch( seq::CompilerError& err ) {
		return;
	}

	FAIL( "Expected exception!" )

} );

TEST( c_fail_stream_double, {

	std::string code = (
			"#exit << \n"
			"<< 2"
			);

	try{
		seq::Compiler::compile( code );
	}catch( seq::CompilerError& err ) {
		return;
	}

	FAIL( "Expected exception!" )

} );

TEST( ce_value_cast, {

	std::string code = (
			"#return << #1 << null\n"
			"#return << #0.5 << null\n"
			"#return << #\"hi\" << null\n"
			"#return << #null << 123\n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getDouble(), 0.5 );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::String );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Null );

} );

TEST( ce_expression_new, {

	std::string code = (
			"#return << ( null = null ) << ( number = bool ) << ( type = type ) << ( 3 != null )"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(0).Bool().getBool(), true );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(1).Bool().getBool(), false );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(2).Bool().getBool(), true );

} );

TEST( ce_simple_define, {

	std::string code = (
			"#exit << var\n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;

	exe.define( "var", seq::Stream {
			seq::Generic( new seq::type::Number( false, 12.0 ) ),
			seq::Generic( new seq::type::Number( false, 33.0 ) )
	} );
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 12l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 33l );

} );

TEST( c_comments, {

	std::string code = (
			"// test \n"
			"#exit << #{ \n"
			"	#return << // test //111 \n"
			"	// test \n"
			"} \n"
			);

	try{
		seq::Compiler::compile( code );
	}catch( seq::CompilerError& err ) {
		FAIL( "Unexpected exception!" )
	}

} );

TEST( ce_composite_order, {

	std::string code = (
			"set x << 1 \n"
			"set y << 3 \n"
			"set z << 4 \n"
			"#exit << 0 << x << 2 << y << z \n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 0l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 1l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 2l );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(3).Number().getLong(), 3l );

	CHECK( (byte) res.at(4).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(4).Number().getLong(), 4l );

} );

TEST( ce_flowc_cast, {

	std::string code = (
			"#exit << #number << #[\"1\", \"5\", \"8\"] << \"5\" << \"8\""
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 5l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 8l );

} );

TEST( ce_accessor_complex, {

	// - INFO: order of operation is mathematically incorrect but I don't have time to fix it
	// - ((v :: 0 - 1) + (v :: 1 - 1) * 3) should equal (v :: 0 - 1 + (v :: 1 - 1) * 3) but it doesn't
	// UPDATE: tried fixing it in 1.4.4 but I'm not sure if it didn't break something else

	std::string code = (
			"set v << 2 << 4 << 6 \n"
			"#exit << (v :: 2 + v :: 1) << (v :: 0 + v :: 1 * 3) << (1 + v :: 2 * 2) << (v :: 0 - 1 + (v :: 1 - 1) * 3) \n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 10l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 14l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 13l );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(3).Number().getLong(), 10l );

} );

TEST( ce_accessor_cast, {

	std::string code = (
			"set v << 7 \n"
			"#exit << #(v :: 0) << 3 \n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (long) res.size(), 1l );
	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 7l );

} );

TEST( ce_embedded_streams, {

	std::string code = (
			"#exit << (<< 3 << 4) << 5 \n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 3l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 4l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 5l );

} );

TEST( ce_type_cast, {

	std::string code = (
			"#exit << #string << #type << 3.14 \n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(0).String().getString(), std::string( "type" ) ) {
		FAIL( "Invalid string!" );
	}

} );

TEST( ce_nested_stream_native, {

	std::string code = (
			"set var << (<< exit) << 1 \n"
			"#var << 2"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

} );

TEST( ce_set_empty, {

	std::string code = (
			"set x << 1 \n"
			"set x \n"
			"#exit << x \n"
			"#exit << 42 "
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 42l );

} );

TEST( ce_fail_undefined_var, {

	std::string code = (
			"#exit << var"
			);

	try{

		auto buf = seq::Compiler::compile( code );
		seq::ByteBuffer bb( buf.data(), buf.size() );

		seq::Executor exe;
		exe.execute( bb );

	}catch( seq::RuntimeError& err ) {
		return;
	}

	FAIL( "Expected exception!" )

} );

TEST( ce_negative_numbers, {

	std::string code = (
			"#exit << -1 << #-2 << 0"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), -1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), -2l );

} );

TEST( ce_half_fail_math_modes, {

	std::string code = (
			"#exit << (null = 1234)"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::Null );

	EXPECT_ERR( {
		exe.setStrictMath( true );
		exe.execute( bb );
	} );

} );

TEST( ce_vmcall_passing, {

	std::string code = (
			"#exit << #{ \n"
			"	#return << @ \n"
			"} << exit \n"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	CHECK( (byte) exe.getResult().getDataType(), (byte) seq::DataType::VMCall );

} );

TEST( ce_between_anchors, {

	std::string code = (
			"#exit << 1 << #2 << 3"
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 1l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 2l );

} );

TEST( ce_casts, {

	std::string code = "#exit << (<< #number << false) << (<< #string << false << [true] << exit << number)";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 0l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(1).String().getString(), std::string( "false" ) ) {
		FAIL( "Invalid string, expected 'false'!" );
	}

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(2).String().getString(), std::string( "flowc" ) ) {
		FAIL( "Invalid string, expected 'flowc'!" );
	}

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(3).String().getString(), std::string( "func" ) ) {
		FAIL( "Invalid string, expected 'func'!" );
	}

	CHECK( (byte) res.at(4).getDataType(), (byte) seq::DataType::String );
	CHECK_ELSE( res.at(4).String().getString(), std::string( "type" ) ) {
		FAIL( "Invalid string, expected 'type'!" );
	}

} );

TEST( util_simple, {

	seq::Generic g;

	g = seq::util::newNull();
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Null );

	g = seq::util::newBool( true );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Bool );

	g = seq::util::newNumber( 123.0 );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Number );

	g = seq::util::newArg( 1 );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Arg );

	g = seq::util::newString( "hello" );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::String );

	g = seq::util::newType( seq::DataType::Bool );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Type );

	g = seq::util::newVMCall( seq::type::VMCall::CallType::Exit );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::VMCall );

	g = seq::util::newFunction( nullptr, false );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Func );

	g = seq::util::newExpression( seq::ExprOperator::Addition, nullptr, nullptr );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Expr );

	g = seq::util::newStream( 0, nullptr );
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Stream );

	g = seq::util::newNull();
	CHECK( (byte) g.getDataType(), (byte) seq::DataType::Null );

} );

TEST( buffer_get_section, {

	std::vector<byte> arr;
	seq::BufferWriter bw( arr );

	bw.putByte( 'A' );
	bw.putByte( 'B' );
	bw.putByte( 'C' );
	bw.putByte( 'D' );
	bw.putByte( 'E' );

	seq::ByteBuffer bb1( arr.data(), arr.size() );

	seq::BufferReader br1 = bb1.getReader( 1, 3 );
	seq::ByteBuffer bb2 = br1.getSubBuffer();

	CHECK( br1.nextByte(), (byte) 'B' );
	CHECK( br1.nextByte(), (byte) 'C' );
	CHECK( br1.nextByte(), (byte) 'D' );
	CHECK( br1.hasNext(), false );

	seq::BufferReader br2 = bb2.getReader( 1, 2 );

	CHECK( br2.nextByte(), (byte) 'C' );
	CHECK( br2.nextByte(), (byte) 'D' );
	CHECK( br2.hasNext(), false );

} );

TEST( ce_flowc_range, {

	std::string code = "#exit << #[1:5] << 1 << null << 2 << \"hello\" << 3 << true << 4 << 5 << null";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 3 )

	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), 2l );

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(1).Number().getLong(), 3l );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(2).Number().getLong(), 4l );

} );

TEST( ce_no_return, {

	std::string code = "";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 1 )
	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Null );

} );

TEST( ce_binary_op, {

	std::string code = "#exit << ( ((6 & 2) | 1) ^ 7 )";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 1 )
	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), (long) (((6 & 2) | 1) ^ 7) );

} );

TEST( c_fail_multiple, {

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << [[true]] " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << \"\\g\" " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " set #var << 123 " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << << 123 " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << #[#123] << 123 " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << #[true false] << true " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << #[123:true] << true " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << ( " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << { " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << [ " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << ) " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << } " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << ] " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << a:b: " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << [@] " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << [] " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << [{}] " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << (<< #exit << 123) " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " set { #exit << true } " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << #[12:#34] << 123 " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << ( 123 123 ) " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << ( 123 + 321 + ) " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << ( 123 + + 321 ) " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " #exit << {} " );
	} );

	EXPECT_ERR( {
		seq::Compiler::compile( " load " );
	} );

} );

TEST( ce_long_namespace, {

	std::string code = (
			"set a:b:c:d:e << 123 \n"
			"#exit << a:b:c:d:e"
			);


	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 1 )
	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), (long) 123 );

} );

TEST( ce_flowc_null, {

	std::string code = "#exit << (<< #[null] << null << true) << (<< #[false] << null << false)";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 2 )
	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Null );
	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Bool );

} );

TEST( ce_function_break, {

	std::string code = (
			"#exit << #{ \n"
			"	#break << null \n"
			"	#return << 1234 \n"
			"} << 678");

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 1 )
	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Null );

} );

TEST( ce_expression_strings, {

	std::string code = "#exit << (\"12\" + \"3\") << (\"12\" != \"3\") << (\"12\" = \"3\") << (\"12\" != \"12\") << (\"12\" = \"12\")";

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	seq::Executor exe;
	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 5 )

	CHECK_ELSE( res.at(0).String().getString(), std::string( "123" ) ) {
		FAIL( "Invalid string, expected '123'!" );
	}

	CHECK( (byte) res.at(1).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(1).Bool().getBool(), true );

	CHECK( (byte) res.at(2).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(2).Bool().getBool(), false );

	CHECK( (byte) res.at(3).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(3).Bool().getBool(), false );

	CHECK( (byte) res.at(4).getDataType(), (byte) seq::DataType::Bool );
	CHECK( res.at(4).Bool().getBool(), true );

} );

TEST( c_error_handle, {

	static bool flag;
	flag = true;

	seq::Compiler::setErrorHandle( [] (seq::CompilerError err) {
		flag = false;
	} );

	auto buf = seq::Compiler::compile( "#exit << [#4]" );

	seq::Compiler::setErrorHandle( seq::Compiler::defaultErrorHandle );

	if( flag ) {
		FAIL( "Error handle test failed!" );
	}

} );

TEST( ce_executor_parenting, {

	std::string code = (
			"set a << 42 \n"
			"set b << 13 \n"
			"#exit << #test_func << \"#exit << #inc_func << ((a :: 0) + (b :: 0) + 37)\""
			);

	auto buf = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buf.data(), buf.size() );

	static seq::Executor exe;
	exe = seq::Executor();

	exe.inject( "inc_func", [] (seq::Stream& stream) -> seq::Stream {
		if( stream[0].getDataType() != seq::DataType::Number ) {
			throw seq::RuntimeError("Expected number!");
		}

		double i = stream[0].Number().getDouble() + 1;
		return { seq::util::newNumber( i ) };
	} );

	exe.inject( "test_func", [] (seq::Stream& stream) -> seq::Stream {
		if( stream[0].getDataType() != seq::DataType::String ) {
			throw seq::RuntimeError("Expected string!");
		}

		std::string code = stream[0].String().getString();
		auto buf = seq::Compiler::compile( code );
		seq::ByteBuffer bb( buf.data(), buf.size() );

		seq::Executor local_exe( &exe );
		local_exe.execute( bb );

		return local_exe.getResults();
	} );

	exe.execute( bb );

	auto& res = exe.getResults();

	CHECK( (int) res.size(), (int) 1 );
	CHECK( (byte) res.at(0).getDataType(), (byte) seq::DataType::Number );
	CHECK( res.at(0).Number().getLong(), (long) (42 + 13 + 37 + 1) );

} );

TEST( c_namespace_accessor, {

	auto buf = seq::Compiler::compile( "#exit << (a:b::0)" );
	seq::ByteBuffer bb( buf.data(), buf.size() );

} );

REGISTER_EXCEPTION( seq_compiler_error, seq::CompilerError );
REGISTER_EXCEPTION( seq_internal_error, seq::InternalError );
REGISTER_EXCEPTION( seq_runtime_error, seq::RuntimeError );

BEGIN( VSTL_MODE_LENIENT );
