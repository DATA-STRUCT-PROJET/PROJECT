#include "prompt.hpp"
#include "fs.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

PromptCommand::PromptCommand(PromptCommandEnum type, std::vector<std::string> args) : type(type), args(args)
{
}

PromptCommandEnum PromptCommand::getType()
{
	return type;
}

const std::vector<std::string>& PromptCommand::getArgs()
{
	return args;
}

Prompt::Prompt(std::ostream& os, FileSystem& fs) :  fs(fs), os(os), currentDirectory("/")
{
}

Prompt::~Prompt()
{
}

std::string Prompt::GetPromptString() {
    return "\x1b[31m" + currentDirectory + "\033[0m > ";
}

PromptCommandResultEnum Prompt::process(PromptCommand& command)
{
    fileData_t stat;
    std::string arg;
    vd_size_t fd;
    char buffer[BUFFER_SIZE];

	switch (command.getType()) {
	case CD:
        arg = command.getArgs().at(0);
        
        try {
            stat = fs.stat(arg); 
        } catch (std::runtime_error) {
            return PromptCommandResultEnum::FAILURE;
        }

        if (stat.isDirectory) {
            currentDirectory = arg;
            return PromptCommandResultEnum::SUCCESS;
        } else {
            return PromptCommandResultEnum::FAILURE;
        }
		break;
	case LS:
		for (auto &it : fs.list(currentDirectory)) {
			os << it.name << '\t';
		}
		break;
	case TREE:
		// TODO(ehdgks0627): Build TREE, and print
		break;
	case CAT:
        arg = command.getArgs().at(0);
        fd = fs.open(arg);
        // while (!fs.eof(fd)) { TODO(ehdgks0627)
        //     vd_size_t readSize = fs.read(fd, buffer, BUFFER_SIZE);
        //     os.write(buffer, readSize);
        // }
        fs.close(fd);
        return PromptCommandResultEnum::SUCCESS;
		break;
	case TOUCH:
		arg = command.getArgs().at(0);
        if (fs.create(currentDirectory, arg)) {
            return PromptCommandResultEnum::SUCCESS;
        } else {
            return PromptCommandResultEnum::FAILURE;
        }
		break;
	case RMDIR:
		arg = command.getArgs().at(0);
		if (fs.removeDirecotry(arg)) {
            return PromptCommandResultEnum::SUCCESS;
        } else {
            return PromptCommandResultEnum::FAILURE;
        }
		break;
	case RM:
		arg = command.getArgs().at(0);
		if (fs.remove(arg)) {
            return PromptCommandResultEnum::SUCCESS;
        } else {
            return PromptCommandResultEnum::FAILURE;
        }
		break;
	case MKDIR:
        arg = command.getArgs().at(0);
        if (fs.create(currentDirectory, arg)) {
            return PromptCommandResultEnum::SUCCESS;
        } else {
            return PromptCommandResultEnum::FAILURE;
        }
		break;
	case ECHO:
		// TODO(ehdgks0627): Implement ECHO
		break;
    case NONE:
        break;
	}
    return PromptCommandResultEnum::FAILURE;
}

PromptCommand PomprtCommandPraser::parse(std::string line)
{
	std::istringstream ss(line);
	std::string token;
	std::vector<std::string> args;
	PromptCommandEnum type = NONE;
	while (std::getline(ss, token, ' ')) {
		args.push_back(token);
	}
	if (args.size() == 0) {
		throw; // TODO(ehdgks0627): Handle error
	}

	// Make command lower
	std::string command = args.at(0);
	std::transform(command.begin(), command.end(), command.begin(),
		[](unsigned char c) { return std::tolower(c); });


	if (command == "cd") {
		type = CD;
	}
	else if (command == "ls") {
		type = LS;
	} 
	else if (command == "tree") {
		type = TREE;
	}
	else if (command == "cat") {
		type = CAT;
	}
	else if (command == "touch") {
		type = TOUCH;
	}
	else if (command == "rmdir") {
		type = RMDIR;
	}
	else if (command == "rm") {
		type = RM;
	}
	else if (command == "mkdir") {
		type = MKDIR;
	}
	else if (command == "echo") {
		type = ECHO;
	}
	args.erase(args.begin());

	return PromptCommand(type, args);
}