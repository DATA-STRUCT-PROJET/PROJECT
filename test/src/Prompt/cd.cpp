#include "gtest/gtest.h"

#include "prompt.hpp"

class PromptCD : public ::testing::Test
{
protected:
    PromptCD()
        : m_fs((vd_size_t)65535), m_prompt(std::cout, m_fs)
    {
    }

    void SetUp() override
    {
        m_fs.createFolder(std::string("/folder1"));
        m_fs.createFolder(std::string("/folder1/subfolder1"));
        m_fs.create(std::string("/file1.txt"));
    }

    FileSystem m_fs;
    Prompt m_prompt;
};

TEST_F(PromptCD, basic)
{
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd folder1"));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/folder1");
}

TEST_F(PromptCD, same_directory)
{
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd ."));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/");
}

TEST_F(PromptCD, up_directory)
{
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd folder1"));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/folder1");
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd .."));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/");
}

TEST_F(PromptCD, up_directory_twice)
{
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd folder1"));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/folder1");
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd subfolder1"));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/folder1/subfolder1");
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd .."));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/folder1");
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd .."));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/");
}

TEST_F(PromptCD, unknow_target)
{
    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::FAILURE, m_prompt.process("cd folder3"));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/");
    ASSERT_EQ("cd: folder3: No such file or directory\n", ::testing::internal::GetCapturedStdout());
}
 
TEST_F(PromptCD, file_target)
{
    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::FAILURE, m_prompt.process("cd file1.txt"));
    ASSERT_EQ(m_prompt.getCurrentDirectory(), "/");
    ASSERT_EQ("cd: file1.txt: Not a directory\n", ::testing::internal::GetCapturedStdout());
}