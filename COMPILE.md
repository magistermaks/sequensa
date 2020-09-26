## Compiling Sequensa

This repository contains the main binary (sequensa) and all standard libraries (in /src/std)  
to compile all of them the provided python script can be used:

 * To install sequensa on your machine use `python3 ./build.py`
 * To execute API's unit tests use `python3 ./build.py --test`
 * For aditional info use `python3 ./build.py --help`

##### Manual Linking

If you want to (for some reason) compile Sequensa manualy  
on linux, remember to link the main binary with the `-ldl` flag.