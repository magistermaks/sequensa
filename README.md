## Sequensa API
Single file embeddable Sequensa API 

Read more about Sequensa programming language [here](http://darktree.net/projects/sequensa/)

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