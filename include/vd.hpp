#pragma once

#include <iostream>
#include <vector>

#include <stdlib.h>

#if defined(TEST_ENABLED)
#include "gtest/gtest.h"
#endif

using vd_size_t = size_t;

#define DEFAULT_BLOCK_LEN 4096

class virtualDisk
{
    public:
        virtualDisk(vd_size_t nb_block, vd_size_t block_len);

        virtualDisk(const char *path);

        ~virtualDisk() {
            delete[] _magical;
        }

        //method
        void *__read(vd_size_t block, void *ptr, vd_size_t size, vd_size_t offset = 0);
        size_t __write(vd_size_t block, const void *ptr, vd_size_t len, vd_size_t offset = 0);

        bool __save(const char *path);

    private:

#if defined(TEST_ENABLED)
        FRIEND_TEST(VirtualDisk_build, create_new);
        FRIEND_TEST(VirtualDisk_build, loading_vd);
        FRIEND_TEST(VirtualDisk_build, loading_unknow_file);

        friend class VirtualDisk_write;
        FRIEND_TEST(VirtualDisk_write, write_generic);

        friend class VirtualDisk_read;
        FRIEND_TEST(VirtualDisk_read, read_generic);
#endif

        char *_magical = nullptr;
        vd_size_t _nb_block;
        vd_size_t _blocks_len;
};
