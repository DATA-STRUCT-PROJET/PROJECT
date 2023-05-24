#include "gtest/gtest.h"

#include "prompt.hpp"

class PromptECHO_basic : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
    protected:
        PromptECHO_basic()
            : m_fs(Path), m_prompt(std::cout, m_fs)
        {
        }

        FileSystem m_fs;
        Prompt m_prompt;
        static constexpr char *Path = (char *)"./VD/Promp";
};

TEST_P(PromptECHO_basic, basic_generic)
{
    vd_size_t fd;
    const std::string expect = std::get<1>(GetParam());
    std::string res(expect.size(), '\0');

    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("echo file1.txt " + std::get<0>(GetParam())));
    fd = m_fs.open("file1.txt");
    m_fs.read(fd, res.data(), expect.size());
    m_fs.close(fd);
    ASSERT_EQ(res, expect);
}

INSTANTIATE_TEST_SUITE_P(
    EchoBasicTest,
    PromptECHO_basic,
    testing::Values(
        std::make_tuple("ThisIsABasicTest", "ThisIsABasicTest"),
        std::make_tuple("this is a test", "this\nis\na\ntest"),
        std::make_tuple("\"this is\" \"a test\"", "this is\na test")
    )
);

class PromptECHO_help : public ::testing::TestWithParam<std::string>
{
    protected:
        PromptECHO_help()
            : m_fs(Path), m_prompt(std::cout, m_fs)
        {
        }

        FileSystem m_fs;
        Prompt m_prompt;
        static constexpr char *Path = (char *)"./VD/Promp";
        static constexpr char *Help = (char *)"Usage: echo FILE [DATA]...\nConcatenate DATA(s) to FILE.\n";
};

TEST_P(PromptECHO_help, help_generic)
{
    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("echo " + GetParam()));
    ASSERT_EQ(Help, ::testing::internal::GetCapturedStdout());
}

INSTANTIATE_TEST_SUITE_P(
    EchohelpTest,
    PromptECHO_help,
    testing::Values("-h", "--help")
);

class PromptECHO_error : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
    protected:
        PromptECHO_error()
            : m_fs(Path), m_prompt(std::cout, m_fs)
        {
        }

        FileSystem m_fs;
        Prompt m_prompt;
        static constexpr char *Path = (char *)"./VD/Promp";
};

TEST_P(PromptECHO_error, error_generic)
{
    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::FAILURE, m_prompt.process("cat " + std::get<0>(GetParam())));
    ASSERT_EQ(std::get<1>(GetParam()) + "\n", ::testing::internal::GetCapturedStdout());
}

INSTANTIATE_TEST_SUITE_P(
    EchoErrorTest,
    PromptECHO_error,
    testing::Values(
        std::make_tuple("echo file3.txt", "echo: file3.txt: No such file or directory"),
        std::make_tuple("echo folder1", "folder1: Is a directory")
    )
);
