#pragma once

#include <map>

#include <unistd.h>

#include "vd.hpp"

#define VD_NAN std::string::npos
#define MAX_NUMBER_BLOCK 5
#define CONF "CONF\0"

struct fileData_t {
    char conf[5] = {'C','O','N','F', 0};
    std::string name;
    vd_size_t block[MAX_NUMBER_BLOCK] = {VD_NAN};
    vd_size_t size = 0;
    bool isDirectory = false;
};

struct MagicBlock_t {
    vd_size_t _nb_blocks = 0;
    vd_size_t _blocks_size = 0;
    vd_size_t _nb_used_blocks = 1;
    std::vector<fileData_t> files;
};

class FileSystem
{
    #define MAGICBLOCK_BLOCK 0
    public:
        FileSystem(vd_size_t nb_block, vd_size_t block_len = 1);
        FileSystem(std::string path);
        ~FileSystem();

        bool create(std::string filename);

        vd_size_t open(std::string path); // return fd?? // path is just a filename for now
        void close(std::string path);
        void close(vd_size_t fd);

        vd_size_t write(vd_size_t fd, const void *ptr, vd_size_t len);
        vd_size_t read(vd_size_t fd, char *ptr, vd_size_t len);

        fileData_t stat(std::string path);

        void debug();
        bool save(std::string path) {
            __registerMagicBlock();
            return vd.__save(path.c_str());
        }

        // for promp commands (to review and remake later)

        bool remove(std::string filename) {
            return false;
        }

        bool create(std::string path, std::string filename) {
            return create(filename); // directory not implemented yet
        }

        bool removeDirecotry(std::string filename) {
            return removeDirecotry(); // directory not implemented yet
        }

        bool removeDirecotry(fileData_t current = fileData_t()) {
            return false; // directory not implemented yet
        }

        std::vector<fileData_t> list(std::string filename) {
            return list();
        }

        std::vector<fileData_t> list(fileData_t current = fileData_t()) {
            return _magicBlock.files;
        }

    private:
        void __registerMagicBlock();
        void __getMagicBlock();
        vd_size_t __getBlock();
        void __printFileStat(const fileData_t&);
        vd_size_t __newFd();
        fileData_t& __getFileFromFD(vd_size_t fd);

    private:
        vd_size_t _nextBlock = 1; // MAGICBLOCK_BLOCK + 1 the def of MAGICBLOCK_BLOCK dosent make any sense cuz is must be equal to 0 in every circumstance
        // vd_size_t _nb_max_files;
        virtualDisk vd;
        MagicBlock_t _magicBlock;
        std::map<vd_size_t, fileData_t> _fds;
};