
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
import os
import shutil
import subprocess

syscfg = {}
comcfg = {}
args = {}

# set parsed args
def set_args( a ):
    global args
    args = a
    return a

# set tmp path
def set_tmp_path( path ):
    global tmp_path
    tmp_path = path
    return path

# load compiler config
def load_compiler_config( config, key ):
    global comcfg
    if config[key]["inherit"] == "":
        comcfg = config[key]
    else:
        child = config[key]
        parent = load_compiler_config(config, child["inherit"])
        
        for x in child:
            parent[x] = child[x]
        
        comcfg = parent
    return comcfg

# define exit code checking function
def check_exit_code( code ):
    if code != 0:
        if not args.force:
            print( "\nError: Task returned non-zero exit code!" )
            print( " * Try running with `--force` to supress this error" )
            print( " * Try reporting this to project maintainers" )
            exit()
        else:
            print( "\nWarning: Task returned non-zero exit code!" )
            print( " * Try reporting this to project maintainers" )
        
# load system config
def load_system_config( config, key ):
    global syscfg
    syscfg = config[key]
    return syscfg

# define function used to invoke compiler
def compile( path, cargs = "" ):
    path = localize_path( path )
    target = os.path.splitext( path )[0] + ".o"
    
    command = comcfg["compile"].replace( "$input", path ).replace( "$bin", comcfg["binary"] )
    command = command.replace( "$output", tmp_path + syscfg["sep"] + target )
    command = command.replace( "$args", cargs )
    
    print( "Compiling '." + syscfg["sep"] + path + "' => '" + tmp_path + syscfg["sep"] + target + "'" )
    check_exit_code( run_command( command ) )

# define function used to invoke linker
def link( target, paths, largs = "" ):
    target = localize_path( target )
    
    x = ""
    for p in paths:
        x = x + localize_path( tmp_path + p ) + " "
    
    command = comcfg["link"].replace( "$input", x ).replace( "$bin", comcfg["binary"] )
    command = command.replace( "$output", target )
    command = command.replace( "$libs", comcfg["libs"][ os.name ] )
    command = command.replace( "$args", largs )
    
    print( "Linking '" + target + "'" )
    check_exit_code( run_command( command ) )

# localize given path
def localize_path( pth ):
    if os.name == "nt":
        pth = pth.replace("/", "\\")

    if pth[-1] == '/':
        return pth[:-1]

    return pth

# just some short-hand
def rem_dir( pth ):
    try:
        shutil.rmtree( pth )
    except:
        pass
    
# stolen from github gist
# https://gist.github.com/jotaelesalinas/f809d702e4d3e24b19b77b83c9bf5d9e
def run_command(cmd):
    proc = subprocess.Popen(cmd, shell=True)
    return proc.wait()

# stolen from stackoverflow
# https://stackoverflow.com/a/377028
def test_for_command(program):
    def is_exe(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return True
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return True

    return False

# stolen from stackoverflow
# https://stackoverflow.com/a/63840426
def add_to_path( program_path ):
    if os.name == "nt":
        import winreg
        import ctypes

        try:
            with winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER) as root:
                with winreg.OpenKey(root, "Environment", 0, winreg.KEY_ALL_ACCESS) as key:
                    existing_path_value = winreg.EnumValue(key, 3)[1]
                    new_path_value = existing_path_value + program_path + ";"
                    winreg.SetValueEx(key, "PATH", 0, winreg.REG_EXPAND_SZ, new_path_value)

                    HWND_BROADCAST = 0xFFFF
                    WM_SETTINGCHANGE = 0x1A
                    SMTO_ABORTIFHUNG = 0x0002
                    result = ctypes.c_long()
                    SendMessageTimeoutW = ctypes.windll.user32.SendMessageTimeoutW
                    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, u"Environment", SMTO_ABORTIFHUNG, 5000, ctypes.byref(result),) 
        except:
            return False
            pass
            
    else:
        with open(f"{os.getenv('HOME')}/.bashrc", "a") as bash_file:
            bash_file.write(f'\nexport PATH="{program_path}:$PATH"\n')
        os.system(f". {os.getenv('HOME')}/.bashrc")
        
    return True
