
# MIT License
# 
# Copyright (c) 2020, 2021 magistermaks
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# load stuff
import platform
import os
import shutil
import sys
import argparse
import tempfile

# import utils
from utils import *

# get temporary workspace
tmp_path = set_tmp_path( tempfile.gettempdir() + "/seq-tmp" )
project = "Sequensa"

# parse cl args
parser = argparse.ArgumentParser( description="C/C++ build system" )
parser.add_argument( "--test", help=f"run {project} API unit tests", action="store_true" )
parser.add_argument( "--Xalias", help="don't create 'sq' alias", action="store_true" )
parser.add_argument( "--Xpath", help="don't attempt to add sequensa to PATH", action="store_true" )
parser.add_argument( "--compiler", help="specify compiler to use [g++, gcc, clang, msvc]", type=str, default="g++" )
parser.add_argument( "--workspace", help="preserve workspace", action="store_true" )
parser.add_argument( "--uninstall", help=f"uninstall {project}", action="store_true" )
parser.add_argument( "--silent", help="don't ask for confirmations", action="store_true" )
parser.add_argument( "--force", help="ignore task exit codes", action="store_true" )
args = set_args(parser.parse_args())

# define per-compiler configuration
compilers_config = {
    "g++": {
        "inherit": "",
        "compile": "$bin -O3 -g0 -Wall -std=c++11 -c $args -o \"$output\" $input",
        "link": "$bin -std=c++11 $args -o \"$output\" $input $libs",
        "binary": "g++",
        "shared": {
            "compiler": "-fPIC", 
            "linker": "-shared"
        },
        "libs": {
            "posix": "-ldl",
            "nt": ""
        }
    },
    "gcc": {
        "inherit": "g++",
        "binary": "gcc"
    },
    "clang": {
        "inherit": "g++",
        "binary": "clang"
    },
    "msvc": {
        "inherit": "",
        "compile": "cl /O2 /c $args $input /Fo\"$output\" /EHsc",
        "link": "link $args $input /OUT:\"$output\" $libs",
        "binary": "cl",
        "shared": {
            "compiler": "", 
            "linker": "/DLL"
        },
        "libs": {
            "posix": "",
            "nt": ""
        }
    }
}

# define per-system configuration
patform_config = {
    "posix": {
        "exe": "",
        "lib": ".so",
        "path": "",
        "sep": "/"
    },
    "nt": {
        "exe": ".exe",
        "lib": ".dll",
        "path": "C:/sequensa",
        "sep": "\\"
    }
}

if os.name == "posix":
    patform_config["posix"]["path"] = os.getenv('HOME') + "/sequensa"

syscfg = load_system_config(patform_config, os.name)
comcfg = load_compiler_config(compilers_config, args.compiler)

# check selected compiler
if args.compiler not in compilers_config:
    print( "\nError: Compiler not supported!" )
    print(" * Try selecting different compiler using the '--compiler' flag")
    exit()

# print basic info
print( f"{project} builder v2.0" )
print( "Platform: " + platform.system() + ", Selected '" + args.compiler + "' compiler." )

if args.uninstall:
    print( f"\n{project} will be uninstalled from: " + syscfg["path"] )
    
    if not args.silent:
        print( "That dir will be deleted with all Sequensa libraries, do you wish to continue? y/n" )
    
        # exit if user did not select 'yes'
        if input() != "y":
            print( "\nUninstalation aborted!" )
            exit()
        
    # uninstall
    rem_dir( syscfg["path"] )
    
    print( f"\n{project} uninstalled!" )
    exit();

# warn about non-standard compiler
if comcfg["binary"] != "g++":
    print("\nWarning: Selected compiler is non-default!")
    print("Warning: Expected g++, this may cause problems.")
    
# warn about Microsoft being Microsoft
if comcfg["binary"] == "cl":
    print("\nWarning: Selected compiler requires special configuration!")
    print("Warning: Learn more here: docs.microsoft.com/en-us/cpp/build/building-on-the-command-line.")
    
# warn about problems this flag may cause
if args.force:
    print("\nWarning: Using the `--force` flag!")
    print("Warning: Future errors will be ignored.")

# if no compiler avaible exit with error
if not test_for_command( comcfg["binary"] + syscfg["exe"] ):
    print( "\nError: Compiler not found!" )

    if os.name == "posix":
        print( " * Try installing g++ with 'sudo apt install build-essential'" )
    else:
        print( " * Try installing g++ from 'http://mingw.org/'" )

    print(" * Try selecting different compiler using the '--compiler' flag")
    exit()

# build API unit tests
if args.test:
    rem_dir( tmp_path )

    # prepare directories
    try:
        os.mkdir( tmp_path ) 
        os.mkdir( tmp_path + "/src" ) 
        os.mkdir( tmp_path + "/src/api" ) 
    except:
        print( "\nError: Failed to prepare directory structure!" )
        print( " * Try checking installer permissions" )
        exit()

    # compile target
    print( "\nBuilding Target..." )
    compile( "src/api/seqapi.cpp" )
    compile( "src/api/tests.cpp" )
    
    # link target
    print( "\nLinking Target..." )
    link( tmp_path + "/tests" + syscfg["exe"], ["/src/api/seqapi.o", "/src/api/tests.o"] )

    # execute target
    print( "\nRunning Target..." )
    os.system( localize_path( tmp_path + "/tests" + syscfg["exe"] ) )

    # delete tmp directory and exit
    if not args.workspace:
        rem_dir( tmp_path )
    else:
        print( "\nWorkspace: '" + tmp_path + "' preserved." )
        
    exit() 

# warn about target directory
print( f"\n{project} will be installed in: " + syscfg["path"] )

if not args.silent:
    print( "If that dir already exists it will be deleted, do you wish to continue? y/n" )

    # exit if user did not select 'yes'
    if input() != "y":
        print( "\nInstalation aborted!" )
        exit()

# delete directries
rem_dir( syscfg["path"] )
rem_dir( tmp_path )

# prepare directories
try:
    os.mkdir( syscfg["path"] ) 
    os.mkdir( syscfg["path"] + "/lib" ) 
    os.mkdir( syscfg["path"] + "/lib/stdio" ) 
    os.mkdir( syscfg["path"] + "/lib/math" ) 
    os.mkdir( syscfg["path"] + "/lib/meta" ) 
    os.mkdir( syscfg["path"] + "/lib/utils" ) 
    os.mkdir( syscfg["path"] + "/lib/string" ) 
    os.mkdir( syscfg["path"] + "/lib/time" ) 
    os.mkdir( syscfg["path"] + "/lib/system" ) 
    os.mkdir( syscfg["path"] + "/lib/lang" ) 
    os.mkdir( tmp_path ) 
    os.mkdir( tmp_path + "/src" ) 
    os.mkdir( tmp_path + "/src/lib" ) 
    os.mkdir( tmp_path + "/src/std" )
    os.mkdir( tmp_path + "/src/api" ) 
except:
    print( "\nError: Failed to prepare directory structure!" )
    print( " * Try checking installer permissions" )
    exit()
    
# print build status
print( "\nBuilding Targets..." )

# compile all files
fPIC = comcfg["shared"]["compiler"]
compile( "src/api/seqapi.cpp", fPIC )
compile( "src/api/dyncapi.cpp", fPIC )
compile( "src/main.cpp" )
compile( "src/help.cpp" )
compile( "src/build.cpp" )
compile( "src/run.cpp" )
compile( "src/utils.cpp" )
compile( "src/info.cpp" )
compile( "src/decompile.cpp" )
compile( "src/shell.cpp" )
compile( "src/lib/whereami.cpp" )
compile( "src/std/stdio.cpp", fPIC )
compile( "src/std/math.cpp", fPIC )
compile( "src/std/meta.cpp", fPIC )
compile( "src/std/utils.cpp", fPIC )
compile( "src/std/string.cpp", fPIC )
compile( "src/std/time.cpp", fPIC )
compile( "src/std/system.cpp", fPIC )
compile( "src/std/lang.cpp", fPIC )

# print build status
print( "\nLinking Targets..." )

# link targets
shared = comcfg["shared"]["linker"]
link( syscfg["path"] + "/sequensa" + syscfg["exe"], ["/src/api/seqapi.o", "/src/lib/whereami.o", "/src/main.o", "/src/help.o", "/src/build.o", "/src/run.o", "/src/utils.o", "/src/info.o", "/src/decompile.o", "/src/shell.o"] )
link( syscfg["path"] + "/libseqapi" + syscfg["lib"], ["/src/api/seqapi.o", "/src/api/dyncapi.o"], shared )
link( syscfg["path"] + "/lib/stdio/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/stdio.o"], shared )
link( syscfg["path"] + "/lib/math/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/math.o"], shared )
link( syscfg["path"] + "/lib/meta/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/meta.o"], shared )
link( syscfg["path"] + "/lib/utils/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/utils.o"], shared )
link( syscfg["path"] + "/lib/string/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/string.o"], shared )
link( syscfg["path"] + "/lib/time/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/time.o"], shared )
link( syscfg["path"] + "/lib/system/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/system.o"], shared )
link( syscfg["path"] + "/lib/lang/native" + syscfg["lib"], ["/src/api/seqapi.o", "/src/std/lang.o"], shared )

# delete tmp directory
if not args.workspace:
    rem_dir( tmp_path )
else:
    print( "\nWorkspace: '" + tmp_path + "' preserved." )

# add Sequensa to PATH (if not already present)
if not args.Xpath:
    lpath = localize_path( syscfg["path"] )
    if not lpath in os.environ['PATH']:
        if add_to_path( lpath ):
            print( "\nSequensa added to PATH" )
            print( "Please restart shell for changes to take effect" )
        else:
            print( "\nWarning: Failed to add Sequensa to PATH!" )
            print( " * Try checking installer permissions" )
            print( " * Try manually adding Sequensa to PATH" )

# create alias for "sequensa"
if not args.Xalias:
    os.link( syscfg["path"] + "/sequensa" + syscfg["exe"], syscfg["path"] + "/sq" + syscfg["exe"] )

# print done and exit
print( "\nSequensa installation complete!" )

