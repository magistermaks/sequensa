
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

import os
import shutil
import subprocess

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
