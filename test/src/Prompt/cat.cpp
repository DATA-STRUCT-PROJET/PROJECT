#include "gtest/gtest.h"

#include "prompt.hpp"

class PromptCAT_basic : public ::testing::TestWithParam<std::tuple<std::string, std::string, PromptCommandResultEnum>>
{
    protected:
        PromptCAT_basic()
            : m_fs((vd_size_t)65535), m_prompt(std::cout, m_fs)
        {
        }

        void SetUp() override 
        {
            std::vector<std::tuple<std::string, std::string>> metas = {
                std::make_tuple("file1.txt", "this a test file"),
                std::make_tuple("file2.txt", "this another test file"),
            };
            for (const auto& fileMeta : metas) {
                std::string filename, data;
                std::tie(filename, data) = fileMeta;
                m_fs.create(filename);
                vd_size_t fd = m_fs.open(filename);
                m_fs.write(fd, (void*)data.c_str(), data.size());
                m_fs.close(fd);
            }
            m_fs.createFolder(std::string("folder1"));
        }

        FileSystem m_fs;
        Prompt m_prompt;
};

TEST_P(PromptCAT_basic, basic_generic)
{
    std::string input = std::get<0>(GetParam());
    std::string expect = std::get<1>(GetParam()) + "\n";
    PromptCommandResultEnum ret  = std::get<2>(GetParam());
    PromptCommandResultEnum result;

    ::testing::internal::CaptureStdout();
    result = m_prompt.process("cat " + input);
    ASSERT_EQ(ret, result);
    ASSERT_EQ(expect, ::testing::internal::GetCapturedStdout());
}

INSTANTIATE_TEST_SUITE_P(
    CatBasicTest,
    PromptCAT_basic,
    testing::Values(
        std::make_tuple("file1.txt", "this a test file", PromptCommandResultEnum::SUCCESS),
        std::make_tuple("file2.txt", "this another test file", PromptCommandResultEnum::SUCCESS),
        std::make_tuple("file3.txt", "cat: file3.txt: No such file or directory", PromptCommandResultEnum::FAILURE),
        std::make_tuple("folder1", "cat: folder1: Is a directory", PromptCommandResultEnum::FAILURE)
    )
);

class PromptCAT_multi : public ::testing::Test
{
    protected:
        PromptCAT_multi()
            : m_fs((vd_size_t)65535), m_prompt(std::cout, m_fs)
        {
        }

        void SetUp() override 
        {
            std::vector<std::tuple<std::string, std::string>> metas = {
                std::make_tuple("file1.txt", "this a test file"),
                std::make_tuple("file2.txt", "this another test file"),
            };
            for (const auto& fileMeta : metas) {
                std::string filename, data;
                std::tie(filename, data) = fileMeta;
                m_fs.create(filename);
                vd_size_t fd = m_fs.open(filename);
                m_fs.write(fd, (void*)data.c_str(), data.size());
                m_fs.close(fd);
            }
        }

        FileSystem m_fs;
        Prompt m_prompt;
};

TEST_F(PromptCAT_multi, multiple)
{
    const std::string expect = "this a test file\nthis another test file\n";

    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::SUCCESS, m_prompt.process("cat file1.txt file2.txt"));
    ASSERT_EQ(expect, ::testing::internal::GetCapturedStdout());
}

TEST_F(PromptCAT_multi, multiple_error)
{
    const std::string expect = "cat: file3.txt: No such file or directory\nthis a test file\n";

    ::testing::internal::CaptureStdout();
    ASSERT_EQ(PromptCommandResultEnum::FAILURE, m_prompt.process("cat file3.txt file1.txt"));
    ASSERT_EQ(expect, ::testing::internal::GetCapturedStdout());
}