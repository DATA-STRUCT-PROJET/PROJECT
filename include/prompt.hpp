#pragma once

#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <vector>

#include "fs.hpp"

#define BUFFER_SIZE 8 * 1024

class PromptCommand
{
public:
    PromptCommand(std::vector<std::string> args);

    [[nodiscard]] const std::string &getName() const;
    [[nodiscard]] const std::vector<std::string> &getArgs() const;

private:
    std::string _name;
    std::vector<std::string> _args;
};

enum PromptCommandResultEnum
{
    FAILURE,
    SUCCESS,
    ERROR
};

class Prompt
{
public:
    Prompt(std::ostream& os, FileSystem& fs);
    ~Prompt() = default;

    [[nodiscard]] std::string GetPromptString();

    PromptCommandResultEnum process(const std::string &line);

protected:
    static inline std::map<std::string, PromptCommandResultEnum (Prompt::*)(PromptCommand &)> m_prompMap;

    static void generateMap();

    PromptCommandResultEnum fnCd(PromptCommand &command);
    PromptCommandResultEnum fnLs(PromptCommand &command);
    PromptCommandResultEnum fnTree(PromptCommand &command);
    PromptCommandResultEnum fnCat(PromptCommand &command);
    PromptCommandResultEnum fnTouch(PromptCommand &command);
    PromptCommandResultEnum fnRmdir(PromptCommand &command);
    PromptCommandResultEnum fnRm(PromptCommand &command);
    PromptCommandResultEnum fnMkdir(PromptCommand &command);
    PromptCommandResultEnum fnEcho(PromptCommand &command);

private:
    [[nodiscard]] static PromptCommand parse(const std::string &line);

    FileSystem &fs;
    std::ostream &os;
    std::string currentDirectory{};
};