#include "fs.hpp"
#include <iostream>
#include <sstream>

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

FileSystem::FileSystem(vd_size_t nb_block, vd_size_t block_len)
    : vd(virtualDisk(nb_block, block_len))
{
    std::cout << nb_block << " " << block_len * 1024 << std::endl;
    _magicBlock._nb_block = nb_block;
    _magicBlock._blocks_len = block_len * 1024;
    _nb_max_files = nb_block - 1;

    fileData_t mainFolder;
    mainFolder.name = ".";
    mainFolder.block = MAINFOLDER_BLOCK;
    mainFolder.isDirectory = true;

    __registerFile(mainFolder);
    __registerMagicBlock();
}

FileSystem::FileSystem(char *path) : vd(virtualDisk(path))
{
    __getMagicBlock();
    _nb_max_files = _magicBlock._nb_block - 1;
    // debug();
}

FileSystem::~FileSystem()
{
}

void FileSystem::__registerFile(fileData_t folder)
{
    vd.__write(folder.block, &folder, SIZEOF_FILEDADATA);
    vd.__write(folder.block, folder.files.data(), folder.files.size() * sizeof(fileData_t), SIZEOF_FILEDADATA);
}

fileData_t FileSystem::__getFolder(vd_size_t block)
{
    fileData_t folder;
    vd.__read(block, &folder, SIZEOF_FILEDADATA);
    folder.files.resize(folder.nb_files);
    vd.__read(block, folder.files.data(), folder.nb_files * sizeof(fileData_t), SIZEOF_FILEDADATA);

    return folder;
}

fileData_t FileSystem::__getFolder(std::string path, fileData_t folder)
{
    if (folder.block == (vd_size_t)-1)
        folder = __getFolder(MAINFOLDER_BLOCK);

    if (path.empty()) return folder;
    auto nPath = getBefore(path, '/');

    for (auto &it : folder.files) {
        if (it.name == nPath && it.isDirectory) {
            return __getFolder(getAfter(path, '/'), it);
        }
    }
    return fileData_t();
}

bool FileSystem::create(std::string path, std::string filename)
{
    return __create(path, filename, false);
}

bool FileSystem::createDirectory(std::string path, std::string filename)
{
    return __create(path, filename, true);
}

bool FileSystem::__create(std::string path, std::string filename, bool isDirectory)
{
    fileData_t file;
    file.path = path;
    file.name = filename;
    file.isDirectory = isDirectory;
    file.block = __getBlock();

    fileData_t folder = __getFolder(path);
    folder.files.push_back(file);
    folder.nb_files++;

    __registerFile(file);
    __registerFile(folder);

    __registerMagicBlock();
    return true;
}

bool FileSystem::remove(std::string dirname)
{

    return false;
}

bool FileSystem::removeDirecotry(std::string dirname)
{

    return false;
}

std::vector<fileData_t> FileSystem::list(std::string path)
{
    return __getFolder(path).files;
    // TODO(ehdgks0627): Should return reference?
}

vd_size_t FileSystem::open(std::string path)
{
    // for (auto it : _magicBlock.files) {
    //     if (it.name == path) {
    //         auto fd = __newFd();
    //         _fds[fd] = it;
    //         return fd;
    //     }
    // }
    // std::cerr << "fd failed for file:\t " << path << std::endl;
    // return -1;
    return -1;
}

void FileSystem::debug()
{
    std::cout << "magic block:" << std::endl;
    std::cout << "nb block:\t" << _magicBlock._nb_block << std::endl;
    std::cout << "blocks_len :\t" << _magicBlock._blocks_len << std::endl;
    std::cout << "nb_files :\t" << _magicBlock.nb_files << std::endl;
    std::cout << "nb file max:\t" << _nb_max_files << std::endl;

    __printFileStat(__getFolder(""));
    for (const auto &it : __getFolder("").files)
        __printFileStat(it);
}

vd_size_t FileSystem::write(vd_size_t fd, void *ptr, vd_size_t len)
{
    // fileData_t file = __getFileFromFD(fd);

    // if (file.block == (vd_size_t)-1) return 0;

    // if (len + sizeof(fileData_t) > _magicBlock._blocks_len)
    //     len = _magicBlock._blocks_len - sizeof(fileData_t);
    // file.size = len;

    // vd.__write(file.block, &file, sizeof(fileData_t));
    // vd.__write(file.block, ptr, len, sizeof(fileData_t));

    // for (auto &it : _magicBlock.files) {
    //     if (it.block == file.block)
    //         it = file;
    // }
    // return len;
    return -1;
}


vd_size_t FileSystem::read(vd_size_t fd, void *ptr, vd_size_t len)
{
    // fileData_t file = __getFileFromFD(fd);

    // if (file.block == (vd_size_t)-1) return 0;

    // if (len + sizeof(fileData_t) > _magicBlock._blocks_len)
    //     len = _magicBlock._blocks_len - sizeof(fileData_t);

    // vd.__read(file.block, ptr, len, sizeof(fileData_t));

    // return len;
    return -1;
}

fileData_t FileSystem::stat(std::string path)
{
    for (auto &it : __getFolder("").files) {
        if (it.name == path) {
            return it;
        }
    }
    throw std::runtime_error("file not found");
}

/*------------------------------------------------------------------*/

fileData_t FileSystem::__getFileFromFD(vd_size_t fd)
{
    fileData_t file;
    try {
        file = _fds.at(fd);
    } catch (std::out_of_range &e) {
        file.block = -1;
    }
    return file;
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
    vd.__write(MAGICBLOCK_BLOCK, &_magicBlock, sizeof(MagicBlock_t));
}

void FileSystem::__getMagicBlock()
{
    vd.__read(MAGICBLOCK_BLOCK, &_magicBlock, sizeof(MagicBlock_t));
}

vd_size_t FileSystem::__getBlock()
{
    if (_nextBlock == _nb_max_files + 1)
        return -1;
    _nextBlock++;
    return _nextBlock -1;
}

void FileSystem::__printFileStat(const fileData_t &file)
{
    std::cout << "file name=" << file.name;
    std::cout << "\t file size=" << file.size;
    std::cout << "\tblok used=" << file.block << std::endl;
}

void FileSystem::close(vd_size_t fd)
{
    // TODO(ehdgks0627)
}