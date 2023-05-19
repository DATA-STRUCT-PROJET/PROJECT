#pragma once
#include <iostream>
#include <stdlib.h>
#include <vector>

typedef size_t vd_size_t;

#define DEFAULT_BLOCK_SIZE 4096

class virtualDisk
{
    public:
        virtualDisk(vd_size_t nb_block, vd_size_t block_len);

        virtualDisk(const char *path);

        ~virtualDisk() {
            delete[] _magical;
        }

        //method
        void *__read(vd_size_t block, void* ptr, vd_size_t size, vd_size_t offset = 0);
        size_t __write(vd_size_t block, void* ptr, vd_size_t len, vd_size_t offset = 0);

        bool __save(const char *path);

    private:
        char *_magical = nullptr;
        vd_size_t _nb_block;
        vd_size_t _blocks_len;
};
