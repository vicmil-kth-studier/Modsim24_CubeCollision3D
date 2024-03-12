# Vicmil library
## Description
This is a library developed by vicmil, with the purpose to be reused in multiple projects.

Compatability is very important, you should only have to write the code once and then run it anywhere. This library can run on both linux, mac, windows AND is compilable with emscripten to run in the browser

## Usage
The library is a headers only library, which means that you don't need anything fancy to get started

The library is split into 3 parts, with regards to what dependencies are required

### N1_vicmil_std_lib:
    This just assumes you have the standard library that comes with basically any c++ compiler(C++ 11 standard)
    Since it is a headers only library you just have to include the vicmil_std_lib.h file and you should be good to go

### N2_vicmil_glm:
    This assumes you have glm installed, another headers only library that contains linear algebra. 

### N3_vicmil_opengl:
    This assumes you have SDL2 as well as OpenGL 1.0 installed in order to work. With this library you can do 3D graphics, handle user input etc. If you want to compile for the browser, make sure you have emscripten installed!