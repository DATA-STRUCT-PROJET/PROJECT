#pragma once
#include <unistd.h>
#include <map>
#include "vd.hpp"

#define getBefore(x, y) x.substr(0, x.find_first_of(y))
#define getAfter(x, y) (x.find_first_of(y) != std::string::npos) ? x.substr(x.find_first_of(y) + 1) : ""

class FileSystem
{
    #define MAGICBLOCK_BLOCK 0
    #define MAINFOLDER_BLOCK 1
    public:
        FileSystem(vd_size_t nb_block, vd_size_t block_len = 4);
        FileSystem(char *path);
        ~FileSystem();

        bool create(std::string path, std::string filename, bool isDirectory = false);
        vd_size_t open(std::string path); // return fd?? // path is just a filename for now
        void close(std::string path);
        void close(vd_size_t fd);

        vd_size_t write(vd_size_t fd, void *ptr, vd_size_t len);
        vd_size_t read(vd_size_t fd, void *ptr, vd_size_t len);
        fileData_t stat(std::string path);

        void debug();
        bool save(char *path) {
            __registerMagicBlock();
            return vd.__save(path);
        }

    private:
        void __registerMagicBlock();
        void __getMagicBlock();
        vd_size_t __getBlock();
        void __printFileStat(const fileData_t&);
        vd_size_t __newFd();
        fileData_t __getFileFromFD(vd_size_t fd);

        // void __saveFolder(fileData_t folder);
        fileData_t __getFolder(vd_size_t block);
        fileData_t __getFolder(std::string path, fileData_t folder = fileData_t());

        void __registerFile(fileData_t file);

    private:
        vd_size_t _nextBlock = MAINFOLDER_BLOCK + 1;
        vd_size_t _nb_max_files;
        virtualDisk vd;
        MagicBlock_t _magicBlock;
        std::map<vd_size_t, fileData_t> _fds;
};