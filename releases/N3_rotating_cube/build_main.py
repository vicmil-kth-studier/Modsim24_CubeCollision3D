import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[0])) 

import build

builder = build.CppBuilder()

current_path = build.path_traverse_up(__file__, 0)
build.change_active_directory(build.path_traverse_up(__file__, 0))


builder.setup_emscripten_compiler(use_openGL=True, exported_functions='''"_main, _set_screen_size"''')
builder.cpp_files.append(current_path + "/main.cpp")
builder.other_args.append("-D USE_DEBUG")
builder.other_args.append("""-DDEBUG_KEYWORDS='"main()"'""")

builder.output_file = current_path + "/run.html"

print("build!")
builder.build()

build.launch_html_page(current_path + "/fullscreen.html")

build.run_command("./a.out")

