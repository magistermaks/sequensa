
import platform, os, shutil, sys

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
print( "Sequensa will be installed in: " + path )
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

# stolen from stackoverflow
def add_to_path( program_path:str ):
    if os.name == "nt": # Windows systems
        import winreg # Allows access to the windows registry
        import ctypes # Allows interface with low-level C API's

        with winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER) as root: # Get the current user registry
            with winreg.OpenKey(root, "Environment", 0, winreg.KEY_ALL_ACCESS) as key: # Go to the environment key
                existing_path_value = winreg.EnumValue(key, 3)[1] # Grab the current path value
                new_path_value = existing_path_value + program_path + ";" # Takes the current path value and appends the new program path
                winreg.SetValueEx(key, "PATH", 0, winreg.REG_EXPAND_SZ, new_path_value) # Updated the path with the updated path

            # Tell other processes to update their environment
            HWND_BROADCAST = 0xFFFF
            WM_SETTINGCHANGE = 0x1A
            SMTO_ABORTIFHUNG = 0x0002
            result = ctypes.c_long()
            SendMessageTimeoutW = ctypes.windll.user32.SendMessageTimeoutW
            SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, u"Environment", SMTO_ABORTIFHUNG, 5000, ctypes.byref(result),) 
    else: # If system is *nix
        with open(f"{os.getenv('HOME')}/.bashrc", "a") as bash_file:  # Open bashrc file
            bash_file.write(f'\nexport PATH="{program_path}:$PATH"\n')  # Add program path to Path variable
        os.system(f". {os.getenv('HOME')}/.bashrc")  # Update bash source

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

link( path + "/Sequensa" + exe_ext, ["/src/lib/whereami.o", "/src/main.o", "/src/help.o", "/src/build.o", "/src/run.o", "/src/utils.o"] )
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


print( "\nSequensa installation complete!" )





