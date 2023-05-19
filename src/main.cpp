
#include <fstream>
#include <sstream>

#include "fs.hpp"
#include "prompt.hpp"

int main(int argc, char* argv[]) {
    FileSystem fs((vd_size_t)65535);
    if (argc > 1)
        fs = FileSystem(argv[1]);
    Prompt prompt = Prompt(std::cout, fs);
    std::string line;

    while (true) {
        std::cout << prompt.GetPromptString();
        std::getline(std::cin, line);
        PromptCommandResultEnum result = prompt.process(line);
    }
    return 0;
}