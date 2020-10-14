
# load stuff
import platform
import os
import shutil
import sys
import argparse

# import utils
from utils import *

# parse cl args
parser = argparse.ArgumentParser( description="Sequensa build system" )
parser.add_argument( "--test", help="Run Sequensa API unit tests", action="store_true" )
parser.add_argument( "--na", help="Don't create 'sq' alias", action="store_true" )
parser.add_argument( "--np", help="Don't attempt to add sequensa to PATH", action="store_true" )
parser.add_argument( "--dev", help="Compile in debug mode", action="store_true" )
args = parser.parse_args()

# palatform independent settings
system_name = platform.system()
command = "g++"
tmp_path = "./builder-tmp"
builder_ver = "1.0"

path = ""
linker_args = ""
exe_ext = ""
lib_ext = ""
compiler_args = ""

# set compiler args
if args.dev:
    compiler_args = " -O0 -g3 -Wall -c "
else:
    compiler_args = " -O3 -g0 -Wall -c "

# palatform dependent settings
if system_name == "Linux":
    path = os.getenv('HOME') + "/sequensa"
    linker_args = " -ldl"
    exe_ext = ""
    lib_ext = ".so"
else:
    path = "C:/sequensa"
    linker_args = ""
    exe_ext = ".exe"
    lib_ext = ".dll"

# define function used to invoke compiler
def compile( path, args = "" ):
    print( "Compiling '" + path + ".cpp' => '" + tmp_path + "/" + path + ".o'" )
    os.system( command + compiler_args + args + "-o \"" + tmp_path + "/" + path + ".o\" " + path + ".cpp" ) 

# define function used to invoke linker
def link( target, paths, args = "" ):
    x = ""
    for p in paths:
        x = x + tmp_path + p + " "
	
    print( "Linking '" + target + "'" )
    os.system( command + args + " -o \"" + target + "\" " + x + linker_args ) 

# print basic info
print( "Sequensa builder v" + builder_ver )
print( "Platform: " + system_name + ", Selected '" + command + "' compiler." )

# if no compiler avaible exit with error
if not test_for_command( command + exe_ext ):
    print( "\nError: Compiler not found!" )

    if system_name == "Linux":
        print( " * Try installing it with 'sudo apt install build-essential'" )
    else:
        print( " * Try installing it from 'http://mingw.org/'" )

    exit()

# if in "test" mode, compile, link and execute API unit tests
if args.test:
    rem_dir( tmp_path )

    # prepare directories
    try:
        os.mkdir( tmp_path ) 
        os.mkdir( tmp_path + "/src" ) 
        os.mkdir( tmp_path + "/src/api" ) 
    except:
        print( "\nFailed to prepare directory structure!" )
        print( " * Try checking installer permissions" )
        exit()

    # compile target
    print( "\nBuilding Target..." )
    compile( "src/api/tests" )
	
    # link target
    print( "\nLinking Target..." )
    link( tmp_path + "/tests" + exe_ext, ["/src/api/tests.o"] )

    # execute target
    print( "\nRunning Target..." )
    os.system( localize_path( tmp_path + "/tests" + exe_ext ) )

    # delete tmp directory and exit
    rem_dir( tmp_path )
    exit() 

# warn about target directory
print( "\nSequensa will be installed in: " + path )
print( "If that dir already exists it will be deleted, do you wish to continue? y/n" )

# exit if user did not select 'yes'
if input() != "y":
    print( "\nInstalation aborted!" )
    exit()

# print build status
print( "\nBuilding Targets..." )

# delete directries
rem_dir( path )
rem_dir( tmp_path )

# prepare directories
try:
    os.mkdir( path ) 
    os.mkdir( path + "/lib" ) 
    os.mkdir( path + "/lib/stdio" ) 
    os.mkdir( path + "/lib/math" ) 
    os.mkdir( path + "/lib/rand" ) 
    os.mkdir( path + "/lib/meta" ) 
    os.mkdir( path + "/lib/utils" ) 
    os.mkdir( path + "/lib/string" ) 
    os.mkdir( path + "/lib/time" ) 
    os.mkdir( tmp_path ) 
    os.mkdir( tmp_path + "/src" ) 
    os.mkdir( tmp_path + "/src/lib" ) 
    os.mkdir( tmp_path + "/src/std" )
except:
    print( "\nFailed to prepare directory structure!" )
    print( " * Try checking installer permissions" )
    exit()

# compile all Sequensa files
compile( "src/main" )
compile( "src/help" )
compile( "src/build" )
compile( "src/run" )
compile( "src/utils" )
compile( "src/lib/whereami" )
compile( "src/std/stdio", "-fPIC " )
compile( "src/std/math", "-fPIC " )
compile( "src/std/rand", "-fPIC " )
compile( "src/std/meta", "-fPIC " )
compile( "src/std/utils", "-fPIC " )
compile( "src/std/string", "-fPIC " )
compile( "src/std/time", "-fPIC " )

# print build status
print( "\nLinking Targets..." )

# link targets
link( path + "/sequensa" + exe_ext, ["/src/lib/whereami.o", "/src/main.o", "/src/help.o", "/src/build.o", "/src/run.o", "/src/utils.o"] )
link( path + "/lib/stdio/native" + lib_ext, ["/src/std/stdio.o"], " -shared" )
link( path + "/lib/math/native" + lib_ext, ["/src/std/math.o"], " -shared" )
link( path + "/lib/rand/native" + lib_ext, ["/src/std/rand.o"], " -shared" )
link( path + "/lib/meta/native" + lib_ext, ["/src/std/meta.o"], " -shared" )
link( path + "/lib/utils/native" + lib_ext, ["/src/std/utils.o"], " -shared" )
link( path + "/lib/string/native" + lib_ext, ["/src/std/string.o"], " -shared" )
link( path + "/lib/time/native" + lib_ext, ["/src/std/time.o"], " -shared" )

# delete tmp directory
rem_dir( tmp_path )

# add Sequensa to PATH (if not already present)
if not args.np:
    lpath = localize_path( path )
    if not lpath in os.environ['PATH']:
        add_to_path( lpath )
        print( "\nSequensa added to PATH" )
        print( "Please restart shell for changes to take effect" )

# create alias for "sequensa"
if not args.na:
    os.link( path + "/sequensa" + exe_ext, path + "/sq" + exe_ext )

# print done and exit
print( "\nSequensa installation complete!" )


