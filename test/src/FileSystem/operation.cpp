#include "gtest/gtest.h"

#include "fs.hpp"

class FileSystemOperation_basic : public ::testing::Test
{
protected:
    FileSystemOperation_basic() : m_fs((vd_size_t)65535)
    {
    }

    FileSystem m_fs;
};

TEST_F(FileSystemOperation_basic, create)
{
    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_TRUE(m_fs.create("file2"));
    ASSERT_TRUE(m_fs.create("file3"));

    ASSERT_TRUE(m_fs.createFolder("folder1"));
    ASSERT_TRUE(m_fs.createFolder("folder2"));
    ASSERT_TRUE(m_fs.createFolder("folder3"));
}

TEST_F(FileSystemOperation_basic, create_duplicate_currency_folder)
{
    ASSERT_TRUE(m_fs.createFolder("folder1"));
    ASSERT_FALSE(m_fs.createFolder("./folder1"));
}

TEST_F(FileSystemOperation_basic, create_duplicate_root)
{
    ASSERT_TRUE(m_fs.createFolder("/folder1"));
    ASSERT_FALSE(m_fs.createFolder("folder1"));
}

TEST_F(FileSystemOperation_basic, create_tree)
{
    ASSERT_TRUE(m_fs.createFolder("/folder1"));
    ASSERT_TRUE(m_fs.createFolder("/folder1/sub1"));
    ASSERT_TRUE(m_fs.createFolder("/folder1/sub2"));
    ASSERT_TRUE(m_fs.createFolder("/folder1/sub3"));
}

TEST_F(FileSystemOperation_basic, create_tree_duplicate)
{
    ASSERT_TRUE(m_fs.createFolder("/folder1"));
    ASSERT_TRUE(m_fs.createFolder("/folder1/sub1"));
    ASSERT_TRUE(m_fs.createFolder("/folder1/sub2"));
    ASSERT_TRUE(m_fs.createFolder("/folder1/sub3"));
    ASSERT_FALSE(m_fs.createFolder("/folder1/sub3"));
}

TEST_F(FileSystemOperation_basic, create_duplicate)
{
    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_TRUE(m_fs.create("file2"));
    ASSERT_FALSE(m_fs.create("file1"));

    ASSERT_TRUE(m_fs.createFolder("folder1"));
    ASSERT_TRUE(m_fs.createFolder("folder2"));
    ASSERT_FALSE(m_fs.createFolder("folder1"));
}

TEST_F(FileSystemOperation_basic, create_duplicate_file_directory)
{
    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_FALSE(m_fs.createFolder("file1"));

    ASSERT_TRUE(m_fs.createFolder("folder1"));
    ASSERT_FALSE(m_fs.create("folder1"));
}

TEST_F(FileSystemOperation_basic, open)
{
    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_TRUE(m_fs.open("file1"));
}

TEST_F(FileSystemOperation_basic, open_non_exist)
{
    ASSERT_EQ(m_fs.open("file1"), -1);
}

TEST_F(FileSystemOperation_basic, open_folder)
{
    ASSERT_TRUE(m_fs.createFolder("folder1"));
    ASSERT_EQ(m_fs.open("folder1"), -1);
}

TEST_F(FileSystemOperation_basic, open_root_folder)
{
    ASSERT_EQ(m_fs.open("/"), -1);
}

TEST_F(FileSystemOperation_basic, remove)
{
    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_TRUE(m_fs.remove("file1"));

    ASSERT_TRUE(m_fs.createFolder("folder1"));
    ASSERT_TRUE(m_fs.removeFolder("folder1"));
}

TEST_F(FileSystemOperation_basic, remove_non_exist)
{
    ASSERT_FALSE(m_fs.remove("file1"));
    ASSERT_FALSE(m_fs.removeFolder("folder1"));
}

TEST_F(FileSystemOperation_basic, remove_non_folder)
{
    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_FALSE(m_fs.removeFolder("file1"));
}

TEST_F(FileSystemOperation_basic, remove_non_file)
{
    ASSERT_TRUE(m_fs.createFolder("folder1"));
    ASSERT_FALSE(m_fs.remove("folder1"));
}

TEST_F(FileSystemOperation_basic, remove_tree)
{
    ASSERT_TRUE(m_fs.createFolder("/folder1"));
    ASSERT_TRUE(m_fs.createFolder("/folder1/sub1"));
    ASSERT_FALSE(m_fs.removeFolder("sub1"));
    ASSERT_TRUE(m_fs.removeFolder("/folder1/sub1"));
}

TEST_F(FileSystemOperation_basic, write)
{
    vd_size_t fd;
    std::string data("TEST_DATA");
    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_NE((fd = m_fs.open("file1")), -1);
    ASSERT_EQ(m_fs.write(fd, (void *)data.c_str(), data.size()), data.size());
    m_fs.close(fd);
}

TEST_F(FileSystemOperation_basic, read)
{
    vd_size_t fd;
    std::string data("TEST_DATA"), read_data;

    ASSERT_TRUE(m_fs.create("file1"));
    ASSERT_NE((fd = m_fs.open("file1")), -1);
    ASSERT_EQ(m_fs.write(fd, (void *)data.c_str(), data.size()), data.size());
    m_fs.close(fd);

    ASSERT_NE((fd = m_fs.open("file1")), -1);
    read_data.resize(data.size());
    ASSERT_EQ(m_fs.read(fd, read_data.data(), data.size()), data.size());
    ASSERT_EQ(data, read_data);
    m_fs.close(fd);
}

TEST_F(FileSystemOperation_basic, get_file)
{
    ASSERT_TRUE(m_fs.create("file1"));
    fileData_t fileData = m_fs.getFile("file1");
    ASSERT_STREQ(fileData.name, "file1");
    ASSERT_EQ(fileData.size, 0);
}

TEST_F(FileSystemOperation_basic, get_file_non_exist)
{
    EXPECT_THROW({
        try
        {
            fileData_t fileData = m_fs.getFile("file1");
        }
        catch (const std::runtime_error &e)
        {
            EXPECT_STREQ("file1: file not found", e.what());
            throw;
        }
    },
                 std::runtime_error);
}

TEST_F(FileSystemOperation_basic, get_folder)
{
    ASSERT_TRUE(m_fs.createFolder("folder1"));
    dirData_t dirData = m_fs.getFolder("folder1");
    ASSERT_STREQ(dirData.name, "folder1");
    ASSERT_EQ(dirData.files.size(), 0);
}

TEST_F(FileSystemOperation_basic, get_folder_non_exist)
{
    EXPECT_THROW({
        try
        {
            dirData_t dirData = m_fs.getFolder("folder1");
        }
        catch (const std::runtime_error &e)
        {
            EXPECT_STREQ("folder1: folder not found", e.what());
            throw;
        }
    },
                 std::runtime_error);
}

TEST_F(FileSystemOperation_basic, stat_file)
{
    ASSERT_TRUE(m_fs.create("file1"));
    m_fs.stat("file1");
    m_fs.stat("/file1");
    m_fs.stat("//file1");
    m_fs.stat("/./file1");
}

TEST_F(FileSystemOperation_basic, stat_folder)
{
    ASSERT_TRUE(m_fs.createFolder("folder1"));
    m_fs.stat("folder1");
    m_fs.stat("/folder1");
    m_fs.stat("//folder1");
    m_fs.stat("/./folder1");
}

TEST_F(FileSystemOperation_basic, stat_non_exist)
{
    EXPECT_THROW({
        try
        {
            m_fs.stat("file1");
        }
        catch (const std::runtime_error &e)
        {
            EXPECT_STREQ("file1: file not found", e.what());
            throw;
        }
    },
                 std::runtime_error);
}