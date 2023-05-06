#pragma once
#include <iostream>
#include <stdlib.h>
#include <vector>

typedef unsigned short vd_size_t;

#define SIZEOF_FILEDADATA (sizeof(fileData_t) - sizeof(std::vector<fileData_t>))

struct fileData_t {
    std::string path;
    std::string name;
    vd_size_t block = -1;
    vd_size_t size = 0; // only for file
    bool isDirectory = false;
    vd_size_t nb_files = 0; // only for directory
    std::vector<fileData_t> files; // only for directory
};

struct MagicBlock_t {
    vd_size_t _nb_block = 0;
    vd_size_t _blocks_len = 0;
    vd_size_t _nb_blocks_used = 0;
    vd_size_t nb_files = 0;
    // std::vector<fileData_t> files; 
};

class virtualDisk
{
    public:
        virtualDisk(vd_size_t nb_block, vd_size_t block_len) : _nb_block(nb_block), _blocks_len(block_len * 1024) {
            _magical = new char[nb_block * _blocks_len];
            std::cout << nb_block * _blocks_len << std::endl;
            if (_magical == nullptr)
                throw std::runtime_error(std::string("failed to allocated disk memory of size") + std::to_string(nb_block * _blocks_len));
        }

        virtualDisk(char *path);

        ~virtualDisk() {
            delete[] _magical;
        }

        //method
        void *__read(vd_size_t block, void* ptr, vd_size_t size, vd_size_t offset = 0);
        size_t __write(vd_size_t block, void* ptr, vd_size_t len, vd_size_t offset = 0);

        bool __save(char *path);

    private:
        char *_magical = nullptr;
        vd_size_t _nb_block;
        vd_size_t _blocks_len;
};
