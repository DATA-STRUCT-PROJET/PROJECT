#pragma once

#include <map>
#include <unistd.h>
#include "vd.hpp"

#define MAGICBLOCK_BLOCK 0
#define MAIN_FOLDER_BLOCK 1

#define VD_NAN std::string::npos
#define MAX_NUMBER_BLOCK 5
#define FILE_CONF "CONF\0"
#define FOLDER_CONF "COND\0"

struct fileData_t {
    char conf[5] = {'C','O','N','F', 0};
    std::string path = ".";
    std::string name = ".";
    vd_size_t block[MAX_NUMBER_BLOCK] = {VD_NAN};
    vd_size_t size = 0;
};

struct dirData_t {
    char conf[5] = {'C','O','N','D', 0};
    std::string path = ".";
    std::string name = ".";
    vd_size_t block = VD_NAN;
    std::vector<vd_size_t> files;
};

struct fileStat_t {
    std::string path = ".";
    std::string name = ".";
    vd_size_t size = 0;
    bool isFolder = false;
};

struct MagicBlock_t {
    vd_size_t _nb_blocks = 0;
    vd_size_t _blocks_size = 0;
    vd_size_t _nb_used_blocks = 1;
    std::vector<vd_size_t> _free_blocks;
};

class FileSystem
{
    public:
        FileSystem(vd_size_t nb_block, vd_size_t block_len = 1);
        FileSystem(std::string path);
        ~FileSystem() = default;

        bool create(std::string filename);
        bool createFolder(std::string filename);

        vd_size_t open(std::string filename);
        void close(std::string path);
        void close(vd_size_t fd);
        
        vd_size_t write(vd_size_t fd, void *ptr, vd_size_t len);
        vd_size_t read(vd_size_t fd, char *ptr, vd_size_t len);
        fileStat_t stat(std::string path);

        dirData_t getFolder(std::string path, dirData_t parentFolder = dirData_t());
        fileData_t getFile(std::string path);

        void debug(dirData_t parentFolder = dirData_t()) {
            __printMagicBlock();
            __printFolder((parentFolder.block == VD_NAN) ? __getFolder(MAIN_FOLDER_BLOCK) : parentFolder);
        }
        bool save(std::string path) {
            __registerMagicBlock();
            return vd.__save(path.c_str());
        }

        // for promp commands (to review and remake later)

        bool remove(std::string filename);
        bool removeDirectory(std::string filename);

        std::vector<fileStat_t> list(std::string path) {

            dirData_t folder = getFolder(path);

            return list(folder);
        }

        std::vector<fileStat_t> list(dirData_t current = dirData_t()) {
            std::vector<fileStat_t> result;
            if (current.block == VD_NAN) current = __getFolder(MAIN_FOLDER_BLOCK);

            for (auto it: current.files) {
                result.push_back(__stat(it));
            }

            return result;
        }

    private:
        void __registerMagicBlock();
        void __getMagicBlock();

        void __saveFolder(dirData_t folder);
        dirData_t __getFolder(vd_size_t block);
        dirData_t __getFolder(std::string pathconf, dirData_t parentFolder);

        fileData_t __getFile(vd_size_t block);
        void __saveFile(fileData_t File);
        void __remove(vd_size_t block);
        void __remove(std::vector<vd_size_t> block);

        vd_size_t __getBlock();
        vd_size_t __newFd();
        fileData_t& __getFileFromFD(vd_size_t fd);

        // debug
        fileStat_t __stat(vd_size_t block);
        void __print(vd_size_t block);
        void __printMagicBlock();
        void __printFolder(dirData_t folder);
        void __printFile(const fileData_t&);

        //filedata to block convert
        std::vector<fileData_t> getFileFromBlock(std::vector<vd_size_t> blocks);
        std::vector<dirData_t> getFolderFromBlock(std::vector<vd_size_t> blocks);

    private:
        vd_size_t _nextBlock = 2; // MAGICBLOCK_BLOCK + 1 the def of MAGICBLOCK_BLOCK dosent make any sense cuz is must be equal to 0 in every circumstance
        virtualDisk vd;
        MagicBlock_t _magicBlock;
        std::map<vd_size_t, fileData_t> _fds;
};