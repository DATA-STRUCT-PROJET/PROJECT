
#include <fstream>
#include <sstream>

#include "fs.hpp"
#include "prompt.hpp"

int main(int argc, char* argv[]) {
    FileSystem *fs;
    if (argc >= 2) {
        fs = new FileSystem(argv[1]);
    } else {
        fs = new FileSystem((vd_size_t)65535);
    }
    Prompt prompt = Prompt(std::cout, *fs);
    std::string line;

    while (true) {
        std::cout << prompt.GetPromptString();
        std::getline(std::cin, line);
        PromptCommand command = PomprtCommandPraser::parse(line);
        PromptCommandResultEnum result = prompt.process(command);
    }

    delete fs;
    return 0;
}