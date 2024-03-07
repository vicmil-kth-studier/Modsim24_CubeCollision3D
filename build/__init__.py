"""
This is a file with some help functions to build a cpp project,
Like a make file but with the power of python!
"""

# Add directory to path
# import sys; from pathlib import Path; 
# sys.path.append(str(Path(__file__).resolve().parents[0])) 

import sys
import subprocess
import pathlib
from pathlib import Path; 
from typing import List, Dict
import os
import webbrowser

class CppBuilder:
    """
    A class to help build c++ code projects

    ...

    Attributes
    ----------
    compiler_path: str
        The compiler to use, g++, clang etc...
    cpp_files: List[str]
        List of cpp files that will be compiled
    macros: List[str]
        List of macros to use in the compilation, eg. "key=value"
        Can be used for flags or for assigning values
    lib_paths: List[str]
        All the paths where external libraries can be found, only used if you actually use external libraries
    include_paths: List[str]
        All the places where you can find external include files such as .h files, therefore you dont need to specify the path everywhere
    libs: List[str]
        A list of .lib files that will be used in the compilation
    other_args: List[str]
        Any other arguments to use in the compilation

    output_file: str
        The path to the final executable that will be created

    Methods
    -------
    build()
        builds a c++ project based on the arguments stored in the class
    
    """
    def __init__(self):
        self.compiler_path = "g++"
        self.other_args: List[str] = list()
        self.macros: List[str] = list()
        self.include_paths: List[str] = list()
        self.lib_paths: List[str] = list()
        self.libs: List[str] = list()
        self.cpp_files: List = list()

        self.output_file = None


    def build(self) -> None:
        """Build the cpp files with the arguments provided to the class

        Parameters
        ----------

        Returns
        -------
            None
        """
        command = ""
        command += self.compiler_path
        for cpp_file in self.cpp_files:
            command += " " + cpp_file
        for arg_ in self.other_args:
            command += " " + arg_
        for macro in self.macros:
            command += " -D " + macro
        for path in self.include_paths:
            command += " -I " + path
        for path in self.lib_paths:
            command += " -L " + path
        for lib in self.libs:
            command += " -l " + lib
        if self.output_file:
            command += " -o " + self.output_file
        

        run_command(command=command)


    def setup_emscripten_with_opengl(self, exported_functions: str ="_main", use_assets: bool = False):
        """Add the necessary parameters to compile with emscripten, and include OpenGL

        Parameters
        ----------
            exported_functions: str
                List of functions to export from c++ code so they can be invoked from javascript.
                Each exported function needs an additional underscore in front, eg main -> _main

            use_assets: bool
                If False: do nothing
                If True: Include assets folder into the executable. 
                    When the code is run later on, the files in the assets folder can be 
                    accessed as regular files. Note! Requires that the active directory is in 
                    the same directory as a folder called "assets" when compiling.


        Returns
        -------
            None
        """
        current_path = path_traverse_up(__file__, 1)
        self.compiler_path = current_path + "/deps/linux_/emsdk/upstream/emscripten/emcc"

        self.other_args.append("-O2")
        #self.other_args.append("-s USE_SDL=2")
        #self.other_args.append("-s USE_SDL_IMAGE=2")
        #self.other_args.append("-s EXPORTED_FUNCTIONS=" + exported_functions)
        #self.other_args.append("-s EXTRA_EXPORTED_RUNTIME_METHODS=ccall,cwrap")
        #self.other_args.append("""-s SDL2_IMAGE_FORMATS='["png"]'""")
        #self.other_args.append("-s FULL_ES3=1")
        #if use_assets:
        #    self.other_args.append("--preload-file assets")


def path_traverse_up(path: str, count: int) -> str:
    """Traverse the provided path upwards

    Parameters
    ----------
        path (str): The path to start from, tips: use __file__ to get path of the current file
        count (int): The number of directories to go upwards

    Returns
    -------
        str: The path after the traversal, eg "/some/file/path"
    """

    parents = pathlib.Path(path).parents
    path_raw = str(parents[count].resolve())
    return path_raw.replace("\\", "/")


def move_file(src_path: str, dst_path: str) -> None:
    """Move a file from source to destination

    Parameters
    ----------
        src_path (str): Where to copy the file from
        dst_path (str): Where the file should be copied to

    Returns
    -------
        None
    """
    pass


def delete_file(file_path: str) -> None:
    """Delete a file from the computer

    Parameters
    ----------
        file_path (str): The path to the file to delete

    Returns
    -------
        None
    """
    try:
        run_command("rm " + file_path)
    except Exception as e:
        pass


def file_exist(file_path: str) -> bool:
    """Determine if a file exists on the computer

    Parameters
    ----------
        file_path (str): The path to the file

    Returns
    -------
        bool: returns true if the file exist, otherwise false
    """
    return os.path.exists(file_path)


def change_active_directory(path: str) -> None:
    """Change the current active directory

    Parameters
    ----------
        path (str): The path to the new active directory

    Returns
    -------
        None
    """
    os.chdir(path)


def run_command(command: str, get_result_as_str: bool = False) -> None:
    """Run a command in the terminal

    Parameters
    ----------
        command (str): The command to run
    get_result_as_str:
        if false: the output of the command is printed into the terminal in real time but is not returned
        if true: the output of the command is returned as a string when the command completes

    Returns
    -------
        None
    """
    if get_result_as_str:
        command_split: List[str] = command.split(" ")
        command_split = filter(lambda a: a != "", command_split)

        print(command)
        result = subprocess.check_output(command_split)
        result = str(result)
        result = result.replace("\\n", "\n")
        print(result)
        return result
    else:
        print(command)
        subprocess.call(command, shell=True)


def get_platform() -> str:
    """Gets what platform the system is running on

    Parameters
    ----------

    Returns
    -------
        str: The platform. The return values are "windows", "linux", "mac" or "unknown"
    """

    if sys.platform == "win32":
        return "windows"
    
    if sys.platform == "cygwin":
        return "windows"
    
    if sys.platform == "darwin":
        return "mac"
    
    if sys.platform == "linux":
        return "linux"
    
    return "unknown"


def launch_html_page(html_file_path: str):
    """ Start the webbrowser if not already open and launch the html page

    Parameters
    ----------
        html_file_path (str): The path to the html file that should be shown in the browser

    Returns
    -------
        None
    """
    change_active_directory(path_traverse_up(html_file_path, count=0))
    if not (file_exist(html_file_path)):
        print("html file does not exist!")
        return
    
    file_name: str = html_file_path.replace("\\", "/").rsplit("/", maxsplit=1)[-1]
    webbrowser.open("http://localhost:8000/" + file_name, new=0, autoraise=True)
    run_command("python3 -m http.server")

