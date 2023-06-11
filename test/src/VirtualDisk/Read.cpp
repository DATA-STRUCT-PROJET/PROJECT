#include <cstring>

#include "gtest/gtest.h"

#include "vd.hpp"

// missing: crashing test case
//      - when wrting start out of range
//      - when wrinting goes out of range

class VirtualDisk_read : public ::testing::TestWithParam<std::tuple<std::string, vd_size_t, vd_size_t>>
{
    protected:
        VirtualDisk_read()
            : m_vd(NbBlock, SizeBlock)
        {
        }

        void SetUp() override 
        {
            std::string data1("expected data");
            std::string data2("data from other block");
            m_vd.__write(0, (void*)data1.c_str(), data1.size(), 0);
            m_vd.__write(1, (void*)data2.c_str(), data2.size(), 0);
        }

        virtualDisk m_vd;

        static constexpr vd_size_t NbBlock = 2;
        static constexpr vd_size_t SizeBlock = 42;
};

TEST_P(VirtualDisk_read, read_generic)
{
    const std::string reading = std::get<0>(GetParam());
    char ptr[SizeBlock];

    std::memset(&ptr[0], 0, SizeBlock);
    m_vd.__read(std::get<1>(GetParam()), &ptr[0], static_cast<vd_size_t>(reading.size()), std::get<2>(GetParam()));
    ASSERT_STREQ(ptr, reading.c_str());
}

INSTANTIATE_TEST_SUITE_P(
    VirtualDiskTest,
    VirtualDisk_read,
    testing::Values(
        std::make_tuple("expected data", 0, 0),
        std::make_tuple("data", 0, 9),
        std::make_tuple("data from other block", 1, 0),
        std::make_tuple("other block", 1, 10)
    )
);