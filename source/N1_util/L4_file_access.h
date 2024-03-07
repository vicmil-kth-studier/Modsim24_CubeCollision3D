#include "L3_string.h"

std::string read_file_contents(const std::string& filename) {
    std::ifstream file(filename);
    std::string contents;

    if (file.is_open()) {
        // Read the file content into the 'contents' string.
        std::string line;
        while (std::getline(file, line)) {
            contents += line + "\n"; // Add each line to the contents with a newline.
        }
        file.close();
    } else {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
    }

    return contents;
}

std::vector<std::string> read_file_contents_line_by_line(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> contents;

    if (file.is_open()) {
        // Read the file content into the 'contents' string.
        std::string line;
        while (std::getline(file, line)) {
            contents.push_back(line); // Add each line to the contents with a newline.
        }
        file.close();
    } else {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
    }

    return contents;
}