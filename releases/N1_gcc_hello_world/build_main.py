import sys; from pathlib import Path; 
sys.path.append(str(Path(__file__).resolve().parents[2])) 

import build

builder = build.CppBuilder()

current_path = build.path_traverse_up(__file__, 0)

builder.compiler_path = "g++"
builder.cpp_files.append(current_path + "/main.cpp")

builder.output_file = current_path + "/a.out"

builder.build()

build.run_command("./a.out")

