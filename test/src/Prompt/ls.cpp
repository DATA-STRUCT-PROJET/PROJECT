#include <algorithm>
#include <filesystem>
#include <string>

#include "gtest/gtest.h"

#include "prompt.hpp"

class PromptLS : public ::testing::Test
{
    protected:
        PromptLS()
            : m_fs(Path), m_prompt(std::cout, m_fs)
        {
        }

        FileSystem m_fs;
        Prompt m_prompt;

        static constexpr char *Path = (char *)"./VD/Promp";

        static inline const std::vector<std::string> DirAct = { "file1.txt", "file2.txt", "folder1", "folder2" };
        static inline const std::vector<std::string> DirFolder1 = { "file1.txt", "file2.txt" };
};

std::vector<std::string> split(const std::string &_str, char _token)
{
    std::istringstream stream(_str);
    std::string word{};
    std::vector<std::string> res{};

    while (std::getline(stream, word, _token))
        res.push_back(word);
    return res;
}

TEST_F(PromptLS, basic)
{
    PromptCommandResultEnum result;
    std::vector<std::string> lscmd{};

    ::testing::internal::CaptureStdout();
    result = m_prompt.process("ls");
    ASSERT_EQ(result, PromptCommandResultEnum::SUCCESS);

    lscmd = split(::testing::internal::GetCapturedStdout(), '\t');
    std::sort(lscmd.begin(), lscmd.end());

    ASSERT_EQ(lscmd.size(), DirAct.size());
    for (size_t it = 0; it < DirAct.size(); it++)
        ASSERT_EQ(lscmd.at(it), DirAct.at(it));
}

TEST_F(PromptLS, simple_folder)
{
    PromptCommandResultEnum result;
    std::vector<std::string> lscmd{};

    ::testing::internal::CaptureStdout();
    result = m_prompt.process("ls ./folder1");
    ASSERT_EQ(result, PromptCommandResultEnum::SUCCESS);

    lscmd = split(::testing::internal::GetCapturedStdout(), '\t');
    std::sort(lscmd.begin(), lscmd.end());

    ASSERT_EQ(lscmd.size(), DirFolder1.size());
    for (size_t it = 0; it < DirFolder1.size(); it++)
        ASSERT_EQ(lscmd.at(it), DirFolder1.at(it));
}

TEST_F(PromptLS, multiple_folder)
{
    PromptCommandResultEnum result;
    std::vector<std::string> lscmd{};
    std::string word{};
    std::istringstream stream;

    ::testing::internal::CaptureStdout();
    result = m_prompt.process("ls . ./folder1");
    ASSERT_EQ(result, PromptCommandResultEnum::SUCCESS);

    stream.str(::testing::internal::GetCapturedStdout());

    std::getline(stream, word);
    ASSERT_EQ(word, ".:");
    std::getline(stream, word);
    lscmd = split(word, '\t');
    std::sort(lscmd.begin(), lscmd.end());
    ASSERT_EQ(lscmd.size(), DirAct.size());
    for (size_t it = 0; it < DirAct.size(); it++)
        ASSERT_EQ(lscmd.at(it), DirAct.at(it));

    std::getline(stream, word);
    ASSERT_EQ(word, "./folder1:");
    std::getline(stream, word);
    lscmd = split(word, '\t');
    std::sort(lscmd.begin(), lscmd.end());
    ASSERT_EQ(lscmd.size(), DirFolder1.size());
    for (size_t it = 0; it < DirFolder1.size(); it++)
        ASSERT_EQ(lscmd.at(it), DirFolder1.at(it));
}