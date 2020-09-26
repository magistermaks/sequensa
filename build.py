
import platform, os, shutil, sys
from utils import *

system_name = platform.system()
command = ""
path = ""
tmp_path = "./installer-tmp"
compiler_args = " -O0 -g3 -Wall -c "
linker_args = ""
exe_ext = ""
lib_ext = ""

if system_name == "Linux":
	command = "g++"
	path = os.getenv('HOME') + "/sequensa"
	linker_args = " -ldl"
	lib_ext = ".so"
else:
	command = "MinGW"
	path = "C:/sequensa"
	target_ext = ".exe"
	lib_ext = ".dll"

print( "Sequensa advanced installer v1.0" )
print( "Platform: " + system_name + ", Selected '" + command + "' compiler." )

if test_for_command( command ) == None:
	print( "\nError: Compiler not found!" )

	if system_name == "Linux":
		print( " * Try installing it with 'sudo apt install build-essential'" )
	else:
		print( " * Try installing it from 'http://mingw.org/'" )

	exit()

print( "\nSequensa will be installed in: " + path )
print( "If that dir already exists it will be deleted, do you wish to continue? y/n" )

if input() != "y":
	print( "Instalation aborted!" )
	exit()

print( "\nBuilding Targets..." )

try:
	shutil.rmtree( path );
except:
	pass

try:
	shutil.rmtree( tmp_path );
except:
	pass

os.mkdir( path ) 
os.mkdir( path + "/lib" ) 
os.mkdir( path + "/lib/stdio" ) 
os.mkdir( path + "/lib/math" ) 
os.mkdir( tmp_path ) 
os.mkdir( tmp_path + "/src" ) 
os.mkdir( tmp_path + "/src/lib" ) 
os.mkdir( tmp_path + "/src/std" ) 

def compile( path, args = "" ):
	print( "Compiling '" + path + ".cpp' => '" + tmp_path + "/" + path + ".o'" )
	os.system( command + compiler_args + args + "-o \"" + tmp_path + "/" + path + ".o\" " + path + ".cpp" ) 

def link( target, paths, args = "" ):
	x = ""
	for p in paths:
		x = x + tmp_path + p + " "
	
	print( "Linking '" + target + "'" )
	os.system( command + args + " -o \"" + target + "\" " + x + linker_args ) 


compile( "src/main" )
compile( "src/help" )
compile( "src/build" )
compile( "src/run" )
compile( "src/utils" )
compile( "src/lib/whereami" )
compile( "src/std/stdio", "-fPIC " )
compile( "src/std/math", "-fPIC " )

print( "\nLinking Targets..." )

link( path + "/sequensa" + exe_ext, ["/src/lib/whereami.o", "/src/main.o", "/src/help.o", "/src/build.o", "/src/run.o", "/src/utils.o"] )
link( path + "/lib/stdio/native" + lib_ext, ["/src/std/stdio.o"], " -shared" )
link( path + "/lib/math/native" + lib_ext, ["/src/std/math.o"], " -shared" )

try:
	shutil.rmtree( tmp_path );
except:
	pass

if not path in os.environ['PATH']:
	add_to_path( path )
	print( "\nSequensa added to PATH" )
	print( "Please restart shell for changes to take effect" )

os.link( path + "/sequensa" + exe_ext, path + "/seq" + exe_ext )

print( "\nSequensa installation complete!" )





