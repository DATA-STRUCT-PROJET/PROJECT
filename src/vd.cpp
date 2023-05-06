#include "vd.hpp"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

virtualDisk::virtualDisk(char *path)
{
    MagicBlock_t magicBlock;
    std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);

    if (!file.is_open()) throw std::runtime_error("error open the save failed");

    try {
        std::streampos size = file.tellg();
        std::cout << "ok" << size << std::endl;

        _magical = new char[size];
        file.seekg (0, std::ios::beg);
        file.read(_magical, size);
        file.close();

        _nb_block = *(vd_size_t*)_magical;
        _blocks_len = *(vd_size_t*)(_magical + sizeof(vd_size_t));

    } catch(...) {
        throw std::runtime_error("error reading the save");
    }
}

void *virtualDisk::__read(vd_size_t block, void* ptr, vd_size_t size, vd_size_t offset)
{
    vd_size_t pos = block * _blocks_len + offset;
    
    return (void*)memcpy(ptr, ((char*)_magical) + pos, size);
}

size_t virtualDisk::__write(vd_size_t block, void* ptr, vd_size_t len, vd_size_t offset)
{
    vd_size_t pos = block * _blocks_len + offset;
    
    memcpy(((char*)_magical) + pos, ptr, len);
    
    return len;
}

bool virtualDisk::__save(char *path)
{
    std::ofstream file(path, std::ios::out | std::ios::binary);
    
    if (!file.is_open()) return false;
    
    try {
        file.write(_magical, _nb_block * _blocks_len);
        file.close();
    } catch(...) {
        return false;
    }
    
    return true;
}