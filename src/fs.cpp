#include "fs.hpp"
#include <iostream>

#define getFirst(path) path.substr(0, path.find_first_of('/'))
#define getRest(path) ((path.find_first_of('/') == std::string::npos) ? "" : path.substr(path.find_first_of('/') + 1))
#define getLast(path) path.substr(path.find_last_of('/') + 1)
#define getPath(path) path.substr(0, path.find_last_of('/'))

/*-------------------------PUBLIC----------------------------*/

#pragma region Filesystem_public

#pragma region Filesystem_Ctor

FileSystem::FileSystem(vd_size_t nb_block, vd_size_t block_len)
    : vd(virtualDisk(nb_block, block_len))
{
    std::cout << "Creating new Filesystem with " << nb_block << " different blocks with a size of " << block_len * DEFAULT_BLOCK_LEN << " each." << std::endl;
    _magicBlock._nb_blocks = nb_block;
    _magicBlock._blocks_size = block_len * DEFAULT_BLOCK_LEN;

    // create main folder

    dirData_t mainFolder;
    mainFolder.block = MAIN_FOLDER_BLOCK;
    mainFolder.files.clear();
    
    __saveFolder(mainFolder);

    __registerMagicBlock();
}

FileSystem::FileSystem(std::string path) : vd(virtualDisk(path.c_str()))
{
    __getMagicBlock();
}

#pragma endregion

#pragma region create

bool FileSystem::createFolder(std::string filename)
{
    if (_magicBlock._nb_blocks -1 == _magicBlock._nb_used_blocks)
        return false;

    if (filename.back() == '/') filename.erase(filename.end() - 1);

    std::string path = getPath(filename);
    std::string name = getLast(filename);
    path = (path == name) ? "." : path;

    dirData_t parentFolder = getFolder(path);
    dirData_t folder;

    folder.path = path;
    folder.name = name;
    folder.block = __getBlock();
    folder.files.clear();

    parentFolder.files.push_back(folder.block);

    __saveFolder(parentFolder);
    __saveFolder(folder);
    return true;
}

bool FileSystem::create(std::string filename)
{
    if (_magicBlock._nb_blocks -1 == _magicBlock._nb_used_blocks)
        return false;

    if (filename.back() == '/') throw std::invalid_argument("Filename can't end with a '/'");

    std::string path = getPath(filename);
    std::string name = getLast(filename);
    path = (path == name) ? "." : path;

    dirData_t parentFolder = getFolder(path);
    fileData_t file;

    file.name = name;
    std::fill_n(file.block, MAX_NUMBER_BLOCK, VD_NAN);
    file.block[0] = __getBlock();
    parentFolder.files.push_back(file.block[0]);
    
    __saveFolder(parentFolder);
    __saveFile(file);

    return true;
}

#pragma endregion

#pragma region remove

bool FileSystem::remove(std::string filename) {
    return false;
}

bool FileSystem::removeDirectory(std::string filename) {
    return removeDirectory();
}

bool FileSystem::removeDirectory(fileData_t current) {
    return false;
}

#pragma endregion 

#pragma region get_folder_file

dirData_t FileSystem::getFolder(std::string path, dirData_t parentFolder)
{
    if (path == "/") return __getFolder(MAIN_FOLDER_BLOCK);

    if (parentFolder.block == VD_NAN)
        parentFolder = __getFolder(MAIN_FOLDER_BLOCK);

    if (path.back() == '/') path.erase(path.end() - 1);

    std::string current_path = getFirst(path);
    std::string new_path = getRest(path);

    dirData_t folder = __getFolder(current_path, parentFolder);
    if (!new_path.empty()) {
        return getFolder(new_path, folder);
    }
    return folder;
}

fileData_t FileSystem::getFile(std::string filename)
{
    if (filename.back() == '/') throw std::invalid_argument("Filename can't end with a '/'");

    std::string path = getPath(filename);
    std::string name = getLast(filename);
    path = (path == name) ? "." : path;
    
    dirData_t parentFolder = getFolder(path); // change to path folder name

    for (auto it : getFileFromBlock(parentFolder.files)) {
        if (it.name == name) {
            return it;
        }
    }
    throw std::runtime_error((filename + ": file not found").c_str());
}

#pragma endregion

#pragma region open_close_stat

vd_size_t FileSystem::open(std::string filename)
{
    try {
        auto fd = __newFd();
        _fds[fd] = getFile(filename);
        return fd;
    } catch (std::exception &e) {
        std::cerr << "fd failed for file:\t " << filename << std::endl;
        std::cerr << e.what() << std::endl;
    }
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

fileStat_t FileSystem::stat(std::string filename)
{
    try {
        fileData_t file = getFile(filename);
        fileStat_t stat;
        stat.path = file.path;
        stat.name = file.name;
        stat.size = file.size;
        stat.isFolder = false;
        return stat;
    } catch (std::exception &e) {
        try {
            dirData_t folder = getFolder(filename);
            fileStat_t stat;
            stat.path = folder.path;
            stat.name = folder.name;
            stat.size = folder.files.size();
            stat.isFolder = true;
            return stat;
        } catch (std::exception &e) {
            std::cerr << "error: " << filename << " do not exist" << std::endl;
        }
    }
    return fileStat_t(); // can't happen
}

#pragma endregion

#pragma region read_write

vd_size_t FileSystem::write(vd_size_t fd, void *ptr, vd_size_t len)
{
    fileData_t& file = __getFileFromFD(fd);
    vd_size_t tmpLen = 0;

    if (file.block[0] == VD_NAN) return 0;

    if (len + sizeof(fileData_t) > _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t))
        len = _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t);

    file.size = len;

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

    __saveFile(file);

    return len;
}

vd_size_t FileSystem::read(vd_size_t fd, char *ptr, vd_size_t len)
{
    fileData_t file = __getFileFromFD(fd);
    int tmpLen = 0;

    if (file.block[0] == VD_NAN) return 0;

    if (len + sizeof(fileData_t) > _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t))
        len = _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t);

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

    return len;
}

#pragma endregion

#pragma endregion // Filesystem_public

/*-------------------------PRIVATE---------------------------*/

#pragma region Filesystem_private

#pragma region handle_fd_block

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

vd_size_t FileSystem::__getBlock()
{
    if (!_magicBlock._free_blocks.empty()) {
        vd_size_t block = _magicBlock._free_blocks.back();
        _magicBlock._free_blocks.pop_back();
        _magicBlock._nb_used_blocks++;
        return block;
    }
    if (_nextBlock + 1 == _magicBlock._nb_blocks)
        return -1;
    _nextBlock++;
    _magicBlock._nb_used_blocks++;
    return _nextBlock -1;
}

#pragma endregion

#pragma region register_save_magic_block

void FileSystem::__registerMagicBlock()
{
    int sizef = sizeof(dirData_t) - sizeof(std::vector<vd_size_t>);
    vd_size_t offset = std::abs(sizef);

    vd.__write(MAGICBLOCK_BLOCK, &_magicBlock, offset);

    vd_size_t size = _magicBlock._free_blocks.size();
    vd.__write(MAGICBLOCK_BLOCK, &size, sizeof(vd_size_t), offset);
    offset += sizeof(vd_size_t);

    vd.__write(MAGICBLOCK_BLOCK, _magicBlock._free_blocks.data(), size * sizeof(vd_size_t), offset);
}

void FileSystem::__getMagicBlock()
{
    vd.__read(MAGICBLOCK_BLOCK, &_magicBlock, sizeof(MagicBlock_t) - sizeof(std::vector<vd_size_t>));
    
    vd_size_t size = 0;
    vd.__read(MAGICBLOCK_BLOCK, &size, sizeof(vd_size_t), sizeof(MagicBlock_t) - sizeof(std::vector<vd_size_t>));
    
    _magicBlock._free_blocks.resize(size);
    vd.__read(MAGICBLOCK_BLOCK, _magicBlock._free_blocks.data(), size * sizeof(vd_size_t), sizeof(MagicBlock_t) - sizeof(std::vector<vd_size_t>));
}

#pragma endregion

#pragma region get_save_folder_file

void FileSystem::__saveFolder(dirData_t folder)
{

    int sizef = sizeof(dirData_t) - sizeof(std::vector<vd_size_t>);
    vd_size_t offset = std::abs(sizef);
    vd.__write(folder.block, &folder, offset);

    vd_size_t size = folder.files.size();
    vd.__write(folder.block, &size, sizeof(vd_size_t), offset);
    offset += sizeof(vd_size_t);

    vd.__write(folder.block, folder.files.data(), size * sizeof(vd_size_t), offset);
}

dirData_t FileSystem::__getFolder(std::string name, dirData_t parent)
{
    if (name == ".") return parent;

    for (auto it : getFolderFromBlock(parent.files)) {
        if (it.name == name) {
            return it;
        }
    }
    throw std::runtime_error("FileSystem::getFolder(): folder not found");
}

dirData_t FileSystem::__getFolder(vd_size_t block)
{
    dirData_t folder;
    int sizef = sizeof(dirData_t) - sizeof(std::vector<vd_size_t>);
    vd_size_t offset = std::abs(sizef);
    vd.__read(block, &folder, offset);
    
    if (std::strcmp(folder.conf, FOLDER_CONF) != 0) {
        if (std::strcmp(folder.conf, FILE_CONF) == 0) return dirData_t();
        std::cerr << "error block:" << block << " conf= " << folder.conf  << std::endl;
        throw std::runtime_error("FileSystem::getFolderFromBlock(): the block dont refere to a dirData_t");
    }

    vd_size_t size;
    vd.__read(block, &size, sizeof(vd_size_t), offset);
    offset += sizeof(vd_size_t);

    folder.files.resize(size);
    vd.__read(block, folder.files.data(), size * sizeof(vd_size_t), offset);
    
    return folder;
}

std::vector<dirData_t> FileSystem::getFolderFromBlock(std::vector<vd_size_t> blocks)
{
    std::vector<dirData_t> folders;
    for (const auto &block : blocks) {
        dirData_t tmp = __getFolder(block);
        if (tmp.block == VD_NAN) continue;
        folders.push_back(tmp);
    }
    return folders;
}

void FileSystem::__saveFile(fileData_t file)
{
    vd.__write(file.block[0], &file, sizeof(fileData_t));
}

fileData_t FileSystem::__getFile(vd_size_t block)
{
    fileData_t file;
    vd.__read(block, &file, sizeof(fileData_t));
    if (std::strcmp(file.conf, FILE_CONF) != 0) {
        if (std::strcmp(file.conf, FOLDER_CONF) == 0) return fileData_t();
        std::cerr << "error block:" << block << std::endl;
        throw std::runtime_error("FileSystem::getFileFromBlock(): the block dont refere to a filedata");
    }
    return file;
}

std::vector<fileData_t> FileSystem::getFileFromBlock(std::vector<vd_size_t> blocks)
{
    std::vector<fileData_t> files;
    for (const auto &it: blocks) {
        auto tmp = __getFile(it);
        if (tmp.block[0] == VD_NAN) continue;
        files.push_back(tmp);
    }
    return files;
}

#pragma endregion

#pragma region debug

fileStat_t FileSystem::__stat(vd_size_t block)
{
    fileStat_t stat;
    char conf[5];
    vd.__read(block, conf, 5);

    if (strcmp(conf, FILE_CONF) == 0) {
        fileData_t file = __getFile(block);
        stat.path = file.path;
        stat.name = file.name;
        stat.size = file.size;
        stat.isFolder = false;

    } else if (strcmp(conf, FOLDER_CONF) == 0) {
        dirData_t folder = __getFolder(block);
        stat.path = folder.path;
        stat.name = folder.name;
        stat.size = folder.files.size();
        stat.isFolder = true;
    } else {
        throw std::runtime_error("FileSystem::stat(): the block dont refere to a file or folder");
    }
    return stat;
}

void FileSystem::__printMagicBlock()
{
    std::cout << "magic block:" << std::endl;
    std::cout << "nb blocks:\t" << _magicBlock._nb_blocks << std::endl;
    std::cout << "_blocks_size :\t" << _magicBlock._blocks_size << std::endl;
    std::cout << "_nb_used_blocks:\t" << _magicBlock._nb_used_blocks << std::endl;
}

void FileSystem::__print(vd_size_t block)
{
    char conf[5];
    vd.__read(block, conf, 5);

    if (std::strcmp(conf, FILE_CONF) == 0) {
        fileData_t file = __getFile(block);
        __printFile(file);
    } else if (std::strcmp(conf, FOLDER_CONF) == 0) {
        dirData_t folder = __getFolder(block);
        __printFolder(folder);
    } else {
        std::cerr << "error block:" << block << std::endl;
        throw std::runtime_error("FileSystem::print(): the block dont refere to a filedata or folder");
    }
}

void FileSystem::__printFolder(dirData_t folder)
{
    std::cout << "path: " << folder.path << std::endl;
    std::cout << "name: " << folder.name << std::endl;
    std::cout << "block: " << folder.block << std::endl;

    std::cout << "files:" << std::endl;
    for (auto it : folder.files) {
        std::cout << "\nentity of block " << it << std::endl;
        __print(it);
        std::cout << std::endl << std::endl;
    }
}

void FileSystem::__printFile(const fileData_t &file)
{
    std::cout << "file name=" << file.name;
    std::cout << "\t file size=" << file.size;
    std::cout << "\tblok used=";
    for (int i = 0; i < MAX_NUMBER_BLOCK; i++)
        std::cout <<  file.block[i] << ";";
    std::cout << std::endl;
}

#pragma endregion // #pragma region debug

#pragma endregion // #pragma region Filesystem_private

/*-------------------------END PRIVATE-----------------------*/