#define USE_DEBUG
#include "../vicmil_glm.h"

int main() {
    std::cout << "Starting!" << std::endl;
    vicmil::TestClass::run_all_tests({"."});
    std::cout << "Finished!" << std::endl;
}