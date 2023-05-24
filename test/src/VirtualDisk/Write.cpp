#include <cstring>

#include "gtest/gtest.h"

#include "vd.hpp"

// missing: crashing test case
//      - when reading start out of range
//      - when reading goes out of range

class VirtualDisk_write: public ::testing::TestWithParam<std::tuple<std::string, std::string, vd_size_t, vd_size_t>>
{
    protected:
        VirtualDisk_write()
            : m_vd(NbBlock, SizeBlock)
        {
        }

        void SetUp() override
        {
            if (std::get<3>(GetParam()) != 0) {
                std::memcpy(m_vd._magical, "writen data", 11);
                std::memcpy(m_vd._magical + m_vd._blocks_len, "writen data in other block", 26);
            }
        }

        virtualDisk m_vd;

        static constexpr vd_size_t NbBlock = 2;
        static constexpr vd_size_t SizeBlock = 42;
};

TEST_P(VirtualDisk_write, write_generic)
{
    const std::string writing = std::get<0>(GetParam());
    const vd_size_t offset = std::get<2>(GetParam()) * m_vd._blocks_len + std::get<3>(GetParam());
    char ptr[SizeBlock];
    size_t len = 0;

    std::memset(&ptr[0], 0, SizeBlock);
    len = m_vd.__write(std::get<2>(GetParam()), writing.c_str(), static_cast<vd_size_t>(writing.size()), std::get<3>(GetParam()));
    ASSERT_EQ(len, writing.size());
    ASSERT_TRUE(!strncmp(m_vd._magical + offset, writing.c_str(), writing.size()));
}

INSTANTIATE_TEST_SUITE_P(
    VirtualDiskTest,
    VirtualDisk_write,
    testing::Values(
        std::make_tuple("writen data", "writen data", 0, 0),
        std::make_tuple("writen data", "writen writen data", 0, 7),
        std::make_tuple("writen data in other block", "writen data in other block", 1, 0),
        std::make_tuple("writen data in other block", "writen data in writen data in other block", 1, 15)
    )
);