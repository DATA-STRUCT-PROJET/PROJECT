#pragma once

#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <vector>

#if defined(TEST_ENABLED)
#include "gtest/gtest.h"
#endif

#include "fs.hpp"

#define BUFFER_SIZE 8 * 1024

class PromptCommand
{
    public:
        PromptCommand(std::vector<std::string> args);

        [[nodiscard]] const std::string &getName() const;
        [[nodiscard]] const std::vector<std::string> &getArgs() const;

    private:
        std::string m_name;
        std::vector<std::string> m_args;
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
        Prompt(std::ostream &_os, FileSystem &_fs);
        ~Prompt() = default;

        [[nodiscard]] std::string GetPromptString() const;

        PromptCommandResultEnum process(const std::string &_line);

    protected:
        static inline std::map<std::string, PromptCommandResultEnum (Prompt::*)(const PromptCommand &)> m_prompMap;

        static void generateMap();

        PromptCommandResultEnum fnCd(const PromptCommand &_cmd);
        PromptCommandResultEnum fnLs(const PromptCommand &_cmd);
        PromptCommandResultEnum fnTree(const PromptCommand &_cmd);
        PromptCommandResultEnum fnCat(const PromptCommand &_cmd);
        PromptCommandResultEnum fnTouch(const PromptCommand &_cmd);
        PromptCommandResultEnum fnRmdir(const PromptCommand &_cmd);
        PromptCommandResultEnum fnRm(const PromptCommand &_cmd);
        PromptCommandResultEnum fnMkdir(const PromptCommand &_cmd);
        PromptCommandResultEnum fnEcho(const PromptCommand &_cmd);
        PromptCommandResultEnum fnSave(const PromptCommand &_cmd);

    private:
#if defined(TEST_ENABLED)
        FRIEND_TEST(PromptCD, basic);
        FRIEND_TEST(PromptCD, same_directory);
        FRIEND_TEST(PromptCD, up_directory);
        FRIEND_TEST(PromptCD, unknow_target);
        FRIEND_TEST(PromptCD, file_target);
#endif

        [[nodiscard]] static PromptCommand parse(const std::string &line);

        FileSystem &m_fs;
        std::ostream &m_os;
        std::string m_cdir{};
};