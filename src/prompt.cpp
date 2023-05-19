#include "prompt.hpp"
#include "fs.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

PromptCommand::PromptCommand(std::vector<std::string> args) : _args(args)
{
    _name = _args.at(0);
    _args.erase(_args.begin());
}

const std::string &PromptCommand::getName() const
{
    return _name;
}

const std::vector<std::string>& PromptCommand::getArgs() const
{
    return _args;
}

Prompt::Prompt(std::ostream& os, FileSystem& fs) :  fs(fs), os(os), currentDirectory("/")
{
	generateMap();
}

std::string Prompt::GetPromptString() {
    return "\x1b[31m" + currentDirectory + "\033[0m > ";
}

PromptCommandResultEnum Prompt::process(const std::string &line)
{
    PromptCommand cmd = parse(line);

    try {
        for (auto &[_key, _fn] : m_prompMap)
            if (_key == cmd.getName())
                return (this->*_fn)(cmd);
    } catch (std::exception &_e) {
        return PromptCommandResultEnum::FAILURE;
    }
    return PromptCommandResultEnum::ERROR;
}

void Prompt::generateMap()
{
    static bool isGenerated = false;

    if (isGenerated)
        return;
    isGenerated = true;
    m_prompMap["cd"] = &Prompt::fnCd;
    m_prompMap["ls"] = &Prompt::fnLs;
    m_prompMap["tree"] = &Prompt::fnTree;
    m_prompMap["cat"] = &Prompt::fnCat;
    m_prompMap["touch"] = &Prompt::fnTouch;
    m_prompMap["rmdir"] = &Prompt::fnRmdir;
    m_prompMap["rm"] = &Prompt::fnRm;
    m_prompMap["mkdir"] = &Prompt::fnMkdir;
    m_prompMap["echo"] = &Prompt::fnEcho;
}

#pragma region Command function

PromptCommandResultEnum Prompt::fnCd(PromptCommand &command)
{
    fileData_t stat{};

    if (command.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    stat = fs.stat(command.getArgs().at(0));
    if (stat.isDirectory)
        currentDirectory = stat.name;
    return static_cast<PromptCommandResultEnum>(stat.isDirectory);
}

PromptCommandResultEnum Prompt::fnLs(PromptCommand &command)
{
    for (auto &it : fs.list(currentDirectory))
        os << it.name << '\t';
    return PromptCommandResultEnum::SUCCESS;
}

PromptCommandResultEnum Prompt::fnTree(PromptCommand &command)
{
    return PromptCommandResultEnum::FAILURE;
}

PromptCommandResultEnum Prompt::fnCat(PromptCommand &command)
{
    if (command.getArgs().empty() || command.getArgs().size() > 1) return PromptCommandResultEnum::FAILURE; // print usage first??
	if (command.getArgs().front() == "-h" || command.getArgs().front() == "--help")
		return PromptCommandResultEnum::SUCCESS; // print the usage

	std::string filename = command.getArgs().at(0);
    auto fd = fs.open(filename);
	auto stat = fs.stat(filename);
	char *ptr = new char[stat.size + 1]; // adding +1 for the \0

	auto readLen = fs.read(fd, ptr, stat.size);
	ptr[stat.size] = 0; // read dont end with terminated value

	fs.close(fd);
	if (readLen == 0) return PromptCommandResultEnum::FAILURE;

	os << ptr << std::endl;

    return PromptCommandResultEnum::SUCCESS;
}

PromptCommandResultEnum Prompt::fnTouch(PromptCommand &command)
{
    if (command.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    return static_cast<PromptCommandResultEnum>(fs.create(currentDirectory, command.getArgs().at(0)));
}

PromptCommandResultEnum Prompt::fnRmdir(PromptCommand &command)
{
    if (command.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    return static_cast<PromptCommandResultEnum>(fs.removeDirecotry(command.getArgs().at(0)));
}

PromptCommandResultEnum Prompt::fnRm(PromptCommand &command)
{
    if (command.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    return static_cast<PromptCommandResultEnum>(fs.remove(command.getArgs().at(0)));
}

PromptCommandResultEnum Prompt::fnMkdir(PromptCommand &command)
{
    if (command.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    return static_cast<PromptCommandResultEnum>(fs.create(currentDirectory, command.getArgs().at(0)));
}

PromptCommandResultEnum Prompt::fnEcho(PromptCommand &command)
{
	if (command.getArgs().empty() || command.getArgs().size() > 2) return PromptCommandResultEnum::FAILURE; // print usage first??
	if (command.getArgs().front() == "-h" || command.getArgs().front() == "--help")
		return PromptCommandResultEnum::SUCCESS; // print the usage


	std::string buff = command.getArgs().at(0);
	std::string file = command.getArgs().at(1);
	auto fd = fs.open(file);

	if (fd == -1) return PromptCommandResultEnum::FAILURE;

	auto lenWritten = fs.write(fd, buff.data(), buff.size());

	fs.close(fd);

    return (lenWritten > 0) ? PromptCommandResultEnum::SUCCESS :  PromptCommandResultEnum::FAILURE;
}

#pragma endregion

PromptCommand Prompt::parse(const std::string &line)
{
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> args;

    while (std::getline(ss, token, ' ')) {
        args.push_back(token);
    }
    if (args.size() == 0) {
        throw; // TODO(ehdgks0627): Handle error
    }
    std::transform(args.at(0).begin(), args.at(0).end(), args.at(0).begin(), [] (const char c) {
        return std::tolower(c);
    });
    return PromptCommand(args);
}