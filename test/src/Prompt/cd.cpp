#include "gtest/gtest.h"

#include "prompt.hpp"

class PromptCD : public ::testing::Test
{
    protected:
        PromptCD()
            : m_fs(Path), m_prompt(std::cout, m_fs)
        {
        }

        FileSystem m_fs;
        Prompt m_prompt;

        static constexpr char *Path = (char *)"./VD/Promp";
};

TEST_F(PromptCD, basic)
{
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd folder1"));
    ASSERT_EQ(m_prompt.m_cdir, "/folder1");
}

TEST_F(PromptCD, same_directory)
{
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd ."));
    ASSERT_EQ(m_prompt.m_cdir, "/");
}

TEST_F(PromptCD, up_directory)
{
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd folder1"));
    ASSERT_EQ(m_prompt.m_cdir, "/folder1");
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cd .."));
    ASSERT_EQ(m_prompt.m_cdir, "/");
}

TEST_F(PromptCD, unknow_target)
{
    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::FAILURE, m_prompt.process("cd folder3"));
    ASSERT_EQ(m_prompt.m_cdir, "/");
    ASSERT_EQ("cd: folder3: No such file or directory\n", ::testing::internal::GetCapturedStdout());
}

TEST_F(PromptCD, file_target)
{
    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::FAILURE, m_prompt.process("cd file1.txt"));
    ASSERT_EQ(m_prompt.m_cdir, "/");
    ASSERT_EQ("cd: file1.txt: Not a directory\n", ::testing::internal::GetCapturedStdout());
}