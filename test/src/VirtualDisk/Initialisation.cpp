#include "gtest/gtest.h"

#include "vd.hpp"

class VirtualDisk_build : public ::testing::Test
{
    protected:
        static constexpr char *Path = (char *)"./VD/VirtualDisk";

        static constexpr vd_size_t NbBlock = 2;
        static constexpr vd_size_t LenBlock = 42;
        static constexpr vd_size_t LenBlockProc = LenBlock * DEFAULT_BLOCK_SIZE;
        static constexpr vd_size_t SizeBlock = NbBlock * LenBlockProc;
};

TEST_F(VirtualDisk_build, create_new)
{
    virtualDisk vd(NbBlock, LenBlock);

    ASSERT_EQ(vd._nb_block, NbBlock);
    ASSERT_EQ(vd._blocks_len, LenBlockProc);
    ASSERT_NE(vd._magical, nullptr);
}

TEST_F(VirtualDisk_build, loading_vd)
{
    virtualDisk vd(Path);

    ASSERT_EQ(vd._nb_block, NbBlock);
    ASSERT_EQ(vd._blocks_len, LenBlockProc);
    ASSERT_NE(vd._magical, nullptr);
}

TEST_F(VirtualDisk_build, loading_unknow_file)
{
    EXPECT_THROW({
        try {
            virtualDisk vd("UnknowPath/UnknowFile");
        } catch (std::runtime_error &_e) {
            ASSERT_STREQ("error open the save failed", _e.what());
            throw;
        }
    }, std::runtime_error);
}