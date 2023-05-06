#pragma once
#include "fs.hpp"
#include <iostream>
#include <string>
#include <vector>

#define BUFFER_SIZE 8 * 1024

enum PromptCommandEnum
{
    CD,
    LS,
    TREE,
    CAT,
    TOUCH,
    RMDIR,
    RM,
    MKDIR,
    ECHO,
    NONE
};

class PromptCommand
{
public:
    PromptCommand(PromptCommandEnum type, std::vector<std::string> args);

    PromptCommandEnum getType();
    const std::vector<std::string>& getArgs();

private:
    PromptCommandEnum type;
    std::vector<std::string> args;
};

enum PromptCommandResultEnum
{
    ERROR,
    FAILURE,
    SUCCESS,
};

class PomprtCommandPraser
{
public:
    static PromptCommand parse(std::string line);
};

class Prompt
{
public:
    Prompt(std::ostream& os, FileSystem& fs);
    ~Prompt();
    std::string GetPromptString();

    PromptCommandResultEnum process(PromptCommand& command);
private:
    FileSystem& fs;
    std::ostream& os;
    std::string currentDirectory;
};