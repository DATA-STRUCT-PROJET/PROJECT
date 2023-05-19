#pragma once
#include <unistd.h>
#include "vd.hpp"
#include <map>

#define VD_NAN std::string::npos
#define MAX_NUMBER_BLOCK 5
#define CONF "CONF\0"

struct fileData_t {
    char conf[5] = {'C','O','N','F', 0};
    std::string name;
    vd_size_t block[MAX_NUMBER_BLOCK] = {VD_NAN};
    vd_size_t size = 0;
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
        bool remove(std::string filename); // not implemented yet

        vd_size_t open(std::string path); // return fd?? // path is just a filename for now
        void close(std::string path); // not implemented yet
        void close(vd_size_t fd); // not implemented yet
        
        vd_size_t write(vd_size_t fd, void *ptr, vd_size_t len);
        vd_size_t read(vd_size_t fd, char *ptr, vd_size_t len);

        fileData_t stat(std::string path);

        void debug();
        bool save(std::string path) {
            __registerMagicBlock();
            return vd.__save(path.c_str());
        }

    //private:
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