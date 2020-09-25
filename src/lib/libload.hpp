
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
 * On Linux link with -ldl
 * Example: g++ main.o -ldl
 */

/*
 * Usage:
 * 	DynamicLibrary dl( "./path/to/lib" );
 * 	if( dl.isLoaded() ) {
 * 		auto funcptr = dl.fetch<void(int,int)>( "funcname" );
 * 		// ... //
 * 	}else{
 * 		std::cout << dl.getMessage();
 * 		dl.close(); // optional - automatically called by destructor
 * 	}
 */

#ifndef LIB_LIBLOAD_HPP_
#define LIB_LIBLOAD_HPP_

#include <string>

#ifdef LIBLOAD_WIN
#include <windows.h>

struct __DynLibData {
	HINSTANCE handle;
};
#endif

#ifdef LIBLOAD_LINUX
#include <dlfcn.h>

struct __DynLibData {
	void* handle;
};
#endif

class DynamicLibrary {

	public:
		DynamicLibrary( const char* path );
		DynamicLibrary( DynamicLibrary&& dl );
		~DynamicLibrary();

		void close();
		bool isLoaded();
		std::string& getMessage();

		template<typename T>
		T fetch( const char* name );

	private:
		__DynLibData data;
		std::string status = "";

};

#ifdef LIBLOAD_IMPLEMENT

DynamicLibrary::DynamicLibrary( const char* path ) {

#ifdef LIBLOAD_WIN
	this->data.handle = LoadLibrary( TEXT(path) );
	if( !this->data.handle ) this->status = "null handle";
#endif

#ifdef LIBLOAD_LINUX
	this->data.handle = dlopen(path, RTLD_LAZY);
	if( !this->data.handle ) this->status = dlerror();
#endif

}

DynamicLibrary::DynamicLibrary( DynamicLibrary&& dl ) {
	this->data = std::move( dl.data );
	this->status = dl.status;
	dl.status = "library ownership moved";
}

DynamicLibrary::~DynamicLibrary() {
	this->close();
}

std::string& DynamicLibrary::getMessage() {
	return this->status;
}

bool DynamicLibrary::isLoaded() {
	return this->status.empty();
}

void DynamicLibrary::close() {
	if( this->isLoaded() ) {
		this->status = "library unloaded";

#ifdef LIBLOAD_WIN
		FreeLibrary( this->data.handle )
#endif

#ifdef LIBLOAD_LINUX
		dlclose(this->data.handle);
#endif

	}
}

template <typename T> T DynamicLibrary::fetch( const char* name ) {
	if( this->isLoaded() ) {

#ifdef LIBLOAD_WIN
		return reinterpret_cast<T>( GetProcAddress( this->data.handle, name ) );
#endif

#ifdef LIBLOAD_LINUX
		return reinterpret_cast<T>( dlsym( this->data.handle, name ) );
#endif

	}

	return reinterpret_cast<T>(0);
}

#undef LIBLOAD_IMPLEMENT
#endif

#endif /* LIB_LIBLOAD_HPP_ */
