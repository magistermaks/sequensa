## Sequensa

<img src="./logo.png" align="right" alt="Logo" title="Sequensa Logo" width="130.75" height="80" />

__General info:__  Sequensa is a esoteric programing language based on C++ that is good enough to actually use it.

Read more about Sequensa programming language [here](http://darktree.net/projects/sequensa/)  
Learn how to compile/install Sequensa [here](./COMPILE.md)

This repository contains:

 * __Sequensa API__  - single file embeddable C++ Sequensa API
 * __Sequensa Standard Libraries__  - shared libraries injected into Sequensa at runtime
 * __Sequensa__  - application used to compile and run Sequensa programs

#### Sequensa API

The API is located in `/src/api/SeqAPI.hpp` and contains all basic documentation to get you started

```C++
#define SEQ_IMPLEMENT
#include "SeqAPI.hpp"

int main() {

	seq::string code = "#exit << \"Hello World!\""_b;
	
	auto buffer = seq::Compiler::compile( code );
	seq::ByteBuffer bb( buffer.data(), buffer.size() );

	seq::Executor exe;
	exe.execute( bb );
	
	std::cout << exe.getResultString();
	
}
```