#include <algorithm>
#include <filesystem>
#include <string>

#include "gtest/gtest.h"

#include "prompt.hpp"

class PromptLS : public ::testing::Test
{
    protected:
        PromptLS()
            : m_fs((vd_size_t)65535), m_prompt(std::cout, m_fs)
        {
        }

        FileSystem m_fs;
        Prompt m_prompt;

        void SetUp() override
        {
            m_fs.create(std::string("/file1.txt"));
            m_fs.create(std::string("/file2.txt"));
            m_fs.createFolder(std::string("/folder1"));
            m_fs.createFolder(std::string("/folder2"));
            m_fs.create(std::string("/folder1/file1.txt"));
            m_fs.create(std::string("/folder1/file2.txt"));
        }

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

    ::testing::internal::CaptureStdout();
    result = m_prompt.process("ls");
    ASSERT_EQ(result, PromptCommandResultEnum::SUCCESS);

    std::string output = ::testing::internal::GetCapturedStdout();

    for (const std::string& filename : DirAct) {
        ASSERT_NE(output.find(filename), std::string::npos);
    }
}

TEST_F(PromptLS, simple_folder)
{
    PromptCommandResultEnum result;

    ::testing::internal::CaptureStdout();
    result = m_prompt.process("ls ./folder1");
    ASSERT_EQ(result, PromptCommandResultEnum::SUCCESS);

    std::string output = ::testing::internal::GetCapturedStdout();

    for (const std::string& filename : DirFolder1) {
        ASSERT_NE(output.find(filename), std::string::npos);
    }
}