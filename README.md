## Sequensa

<img src="./logo.png" align="right" alt="Logo" title="Sequensa Logo" width="130.75" height="80" />

__General info:__  Sequensa is a esoteric programing language based on C++ stream syntax that is good enough to actually use it.

Read more about Sequensa programming language [here](http://darktree.net/projects/sequensa/)  
Learn how to compile/install Sequensa [here](./COMPILE.md)

This repository contains:

 * __Sequensa API__  - single file embeddable C++ Sequensa API
 * __Sequensa STL__  - shared libraries injected into Sequensa at runtime
 * __Sequensa CLI__  - application used to compile and run Sequensa programs

#### Features
- Interactive shell and extensive CLI
- Compiler and decompiler
- Easy to embed (single file API)
- Simple to use dynamic library interface
- Bindings to different languages ([Java](https://github.com/magistermaks/sequensa-java), [Python](https://github.com/magistermaks/sequensa-python))

#### Sequensa API

The API is located in `/src/api/SeqAPI.hpp` and contains all the basic documentation to get you started

```C++
#define SEQ_IMPLEMENT
#include "SeqAPI.hpp"

int main() {

	std::string code = "#exit << \"Hello World!\"";
	
	auto buffer = seq::Compiler::compileStatic( code );
	seq::ByteBuffer bb( buffer.data(), buffer.size() );

	seq::Executor exe;
	exe.execute( bb );
	
	std::cout << exe.getResultString();
	
}
```
