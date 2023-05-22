#include <cstring>
#include <iostream>

#include "fs.hpp"

FileSystem::FileSystem(vd_size_t nb_block, vd_size_t block_len)
    : vd(virtualDisk(nb_block, block_len))
{
    std::cout << "Creating new Filesystem with " << nb_block << " different blocks with a size of " << block_len * DEFAULT_BLOCK_SIZE << " each." << std::endl;
    _magicBlock._nb_blocks = nb_block;
    _magicBlock._blocks_size = block_len * DEFAULT_BLOCK_SIZE;
    // _nb_max_files = nb_block - 1;
    __registerMagicBlock();
}

FileSystem::FileSystem(std::string path) : vd(virtualDisk(path.c_str()))
{
    __getMagicBlock();
    // _nb_max_files = _magicBlock._nb_blocks - 1;
    // debug();
}

FileSystem::~FileSystem()
{
}

bool FileSystem::create(std::string filename)
{
    if (_magicBlock._nb_blocks -1 == _magicBlock._nb_used_blocks)
        return false;

    fileData_t file;
    // memcpy(file.name, filename, 16);
    file.name = filename;
    std::fill_n(file.block, MAX_NUMBER_BLOCK, VD_NAN);
    file.block[0] = __getBlock();

    _magicBlock.files.push_back(file);
    vd.__write(file.block[0], &file, sizeof(fileData_t));
    __registerMagicBlock();
    return true;
}

vd_size_t FileSystem::open(std::string path)
{
    for (auto &[key, val] : _fds)
        if (val.name == path) return key;

    for (auto it : _magicBlock.files) {
        if (it.name == path) {
            auto fd = __newFd();
            _fds[fd] = it;
            return fd;
        }
    }
    std::cerr << "fd failed for file:\t " << path << std::endl;
    return -1;
}

void FileSystem::close(std::string path) {
    for (auto it = _fds.begin(); it != _fds.end(); it++) {
        if (it->second.name == path) {
            _fds.erase(it);
            break;
        }
    }
    //not found
}

void FileSystem::close(vd_size_t fd) {
    for (auto &[key, _] : _fds)
        if (key == fd) {
            _fds.erase(fd);
            break;
        };
    //not found
}

void FileSystem::debug()
{
    std::cout << "magic block:" << std::endl;
    std::cout << "nb blocks:\t" << _magicBlock._nb_blocks << std::endl;
    std::cout << "_blocks_size :\t" << _magicBlock._blocks_size << std::endl;
    std::cout << "nb_files :\t" << _magicBlock.files.size() << std::endl;
    std::cout << "_nb_used_blocks:\t" << _magicBlock._nb_used_blocks << std::endl;

    for (const auto &it : _magicBlock.files)
        __printFileStat(it);
}

vd_size_t FileSystem::write(vd_size_t fd, const void *ptr, vd_size_t len)
{
    fileData_t& file = __getFileFromFD(fd);
    vd_size_t tmpLen = 0;

    if (file.block[0] == VD_NAN) return 0;

    len += sizeof(fileData_t);

    if (len > _magicBlock._blocks_size * MAX_NUMBER_BLOCK)
        len = _magicBlock._blocks_size * MAX_NUMBER_BLOCK;

    file.size = len - sizeof(fileData_t);
    vd.__write(file.block[0], &file, sizeof(fileData_t));

    len -= sizeof(fileData_t);
    tmpLen = (len > _magicBlock._blocks_size - sizeof(fileData_t)) ? _magicBlock._blocks_size - sizeof(fileData_t) : len;
    vd.__write(file.block[0], ptr, tmpLen, sizeof(fileData_t));

    for (int i = 0; ++i && len > 0 && i < MAX_NUMBER_BLOCK; len -= tmpLen) {
        if (file.block[i] == VD_NAN) {
           file.block[i] = __getBlock(); 
        }
        tmpLen = (len > _magicBlock._blocks_size) ? _magicBlock._blocks_size : len;
        vd.__write(file.block[i], ptr, tmpLen);
    }

    vd.__write(file.block[0], &file, sizeof(fileData_t));

    for (auto &it : _magicBlock.files) {
        if (it.block[0] == file.block[0]) {
            it = file;
            std::cout << "found it" << std::endl;
            __registerMagicBlock();
            break;
        }
    }

    return file.size;
}

vd_size_t FileSystem::read(vd_size_t fd, char *ptr, vd_size_t len)
{
    fileData_t file = __getFileFromFD(fd);
    int tmpLen = 0;

    if (file.block[0] == VD_NAN) return 0;

    if (len + sizeof(fileData_t) > _magicBlock._blocks_size * MAX_NUMBER_BLOCK  - sizeof(fileData_t))
        len = _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t);
    
    vd_size_t lenRead = len;

    tmpLen = (len > _magicBlock._blocks_size - sizeof(fileData_t)) ? _magicBlock._blocks_size - sizeof(fileData_t) : len;
    vd.__read(file.block[0], ptr, tmpLen, sizeof(fileData_t));

    for (int i = 0; ++i && len > 0 && i < MAX_NUMBER_BLOCK; len -= tmpLen) {
        if (file.block[i] == VD_NAN) {
            return -1;
        }
        ptr += tmpLen;
        tmpLen = (len > _magicBlock._blocks_size) ? _magicBlock._blocks_size : len;
        vd.__read(file.block[i], ptr, tmpLen);
    }

    return lenRead;
}

fileData_t FileSystem::stat(std::string path)
{
    for (auto &it : _magicBlock.files) {
        // if (strcmp(it.name, path) == 0) {
        if (it.name == path) {
            return it;
        }
    }
    throw std::runtime_error("file not found");
}

/*------------------------------------------------------------------*/

fileData_t &FileSystem::__getFileFromFD(vd_size_t fd)
{
    try {
        fileData_t& file = _fds.at(fd);
        return file;
    } catch (std::out_of_range &e) {
        throw std::runtime_error("fd not found");
    }
}

vd_size_t FileSystem::__newFd()
{
    if (_fds.size() == 0)
        return 3; // 0,1,2 are use by the system(its not relate to our file descriptor but why not c:)
    
    for (auto key = _fds.begin(), it = _fds.begin().operator++(); it != _fds.end(); it++, key++)
        if (key->first + 1 != it->first)
            return key->first + 1;

    return _fds.rbegin()->first + 1;
}

void FileSystem::__registerMagicBlock()
{
    vd.__write(MAGICBLOCK_BLOCK, &_magicBlock, sizeof(vd_size_t) * 3);
    //vd.__write(MAGICBLOCK_BLOCK, _magicBlock.files.data(), _magicBlock.files.size() * sizeof(fileData_t), sizeof(vd_size_t) * 3);
    
    for(int i = 0; i < _magicBlock.files.size(); i++) {
        vd.__write(MAGICBLOCK_BLOCK, &_magicBlock.files[i], sizeof(fileData_t), sizeof(vd_size_t) * 3 + sizeof(fileData_t) * i);
    }
}

void FileSystem::__getMagicBlock()
{
    vd.__read(MAGICBLOCK_BLOCK, &_magicBlock, sizeof(vd_size_t) * 3);
    _magicBlock.files.resize(_magicBlock._nb_blocks - 1);
    std::cout << "nb block :" << _magicBlock._nb_used_blocks <<std::endl;;

    for (int i = 1; i < _magicBlock._nb_used_blocks; i++) {
        std::cout << "i:" << i << std::endl;
        char tmpPtr[5];
        vd.__read(i, tmpPtr, 5);
        if (std::strcmp(tmpPtr, "CONF\0") == 0) {
            fileData_t tmp;
            vd.__read(i, &tmp, sizeof(fileData_t));
            __printFileStat(tmp);
            _magicBlock.files.push_back(tmp);
        }
        else {
            std::cout << "conf didint found" << std::endl;
        }
    }
}

vd_size_t FileSystem::__getBlock()
{
    if (_nextBlock == _magicBlock._nb_blocks + 1)
        return -1;
    _nextBlock++;
    _magicBlock._nb_used_blocks++;
    return _nextBlock -1;
}

void FileSystem::__printFileStat(const fileData_t &file)
{
    std::cout << "file name=" << file.name;
    std::cout << "\t file size=" << file.size;
    std::cout << "\tblok used=";
    for (int i = 0; i < MAX_NUMBER_BLOCK; i++)
        std::cout <<  file.block[i] << ";";
    std::cout << std::endl;
}