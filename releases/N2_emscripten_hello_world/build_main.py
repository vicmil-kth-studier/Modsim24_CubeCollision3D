import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[0])) 

import build

builder = build.CppBuilder()

current_path = build.path_traverse_up(__file__, 0)
build.change_active_directory(build.path_traverse_up(__file__, 0))


builder.setup_emscripten_with_opengl()
builder.cpp_files.append(current_path + "/main.cpp")

builder.output_file = current_path + "/run.html"

builder.build()

build.launch_html_page(builder.output_file)

build.run_command("./a.out")

