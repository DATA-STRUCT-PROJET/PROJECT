#include "fs.hpp"
#include <iostream>
#include <cstring>

/*-------------------------PUBLIC----------------------------*/

#pragma region Filesystem_public

#pragma region Filesystem_Ctor

FileSystem::FileSystem(vd_size_t nb_block, vd_size_t block_len)
    : vd(virtualDisk(nb_block, block_len))
{
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
    for (auto &_file : list(parentFolder)) {
        if (!name.compare(_file.name)) {
            return false;
        }
    }
    dirData_t folder;

    std::strncpy(folder.path, path.c_str(), 127);
    std::strncpy(folder.name, name.c_str(), 127);
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

    if (filename.back() == '/')
        throw std::invalid_argument("Filename can't end with a '/'");

    std::string path = getPath(filename);
    std::string name = getLast(filename);
    path = (path == name) ? "." : path;

    dirData_t parentFolder = getFolder(path);
    for (auto &_file : list(parentFolder)) {
        if (!name.compare(_file.name)) {
            return false;
        }
    }
    fileData_t file;

    std::strncpy(file.name, name.c_str(), 127);
    std::fill_n(file.block, MAX_NUMBER_BLOCK, VD_NAN);
    file.block[0] = __getBlock();
    parentFolder.files.push_back(file.block[0]);

    __saveFolder(parentFolder);
    __saveFile(file);

    return true;
}

#pragma endregion

#pragma region remove

bool FileSystem::remove(std::string filename)
{
    if (filename.back() == '/') throw std::invalid_argument("Filename can't end with a '/'");

    std::string path = getPath(filename);
    std::string name = getLast(filename);
    path = (path == name) ? "." : path;

    dirData_t parentFolder = getFolder(path);

    for (auto it = parentFolder.files.begin(); it != parentFolder.files.end(); it++) {
        auto file_tmp = __getFile(*it);
        if (file_tmp.name == name) {
            parentFolder.files.erase(it);
            __saveFolder(parentFolder);
            __remove(file_tmp.block[0]); // only modify the first 5 bytes (conf data)
            return true;
        }
    }
    return false;
}

bool FileSystem::removeFolder(std::string filename)
{
    if (filename.back() == '/') filename.erase(filename.end() -1);

    std::string path = getPath(filename);
    std::string name = getLast(filename);
    path = (path == name) ? "." : path;

    dirData_t parentFolder = getFolder(path);

    for (auto it = parentFolder.files.begin(); it != parentFolder.files.end(); it++) {
        auto folder_tmp = __getFolder(*it);
        if (folder_tmp.name == name) {
            if (!folder_tmp.files.empty()) throw std::invalid_argument("Folder is not empty");
            parentFolder.files.erase(it);
            __saveFolder(parentFolder);
            __remove(folder_tmp.block); // only modify the first 5 bytes (conf data)
            return true;
        }
    }
    return false;
}


#pragma endregion 

#pragma region get_folder_file

dirData_t FileSystem::getFolder(std::string path, dirData_t parentFolder)
{
    if (path == "/" || path.empty())
        return __getFolder(MAIN_FOLDER_BLOCK);

    if (parentFolder.block == VD_NAN)
        parentFolder = __getFolder(MAIN_FOLDER_BLOCK);

    if (path.back() == '/') path.erase(path.end() - 1);
    if (path.front() == '/') path.erase(path.begin());

    std::string current_path = getFirst(path);
    std::string new_path = getRest(path);
    dirData_t folder = __getFolder(current_path, parentFolder);

    if (!new_path.empty())
        return getFolder(new_path, folder);
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
    } catch (std::exception &_e) {
        std::ignore = _e;
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
    fileStat_t stat;

    try {
        fileData_t file = getFile(filename);

        std::strcpy(stat.path, file.path);
        std::strcpy(stat.name, file.name);
        stat.size = file.size;
        stat.isFolder = false;
    } catch (std::exception &_e) {
        std::ignore = _e;

        try {
            dirData_t folder = getFolder(filename);

            std::strcpy(stat.path, folder.path);
            std::strcpy(stat.name, folder.name);
            stat.size = folder.files.size();
            stat.isFolder = true;
        } catch (std::exception &_e) {
            std::ignore = _e;

            throw std::runtime_error((filename + ": file not found").c_str());
        }
    }
    return stat;
}

#pragma endregion

#pragma region read_write

// overwrite the data already writen
vd_size_t FileSystem::write(vd_size_t fd, void *ptr, vd_size_t len)
{
    fileData_t& file = __getFileFromFD(fd);
    vd_size_t tmpLen = 0;
    if (file.block[0] == VD_NAN)
        return 0;
        
    file.size = len;

    len = std::min(len + sizeof(fileData_t), _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t));


    len -= sizeof(fileData_t);
    tmpLen = std::min(_magicBlock._blocks_size - sizeof(fileData_t), len);
    vd.__write(file.block[0], ptr, tmpLen, sizeof(fileData_t));

    for (int i = 0; len > 0 && i < MAX_NUMBER_BLOCK; len -= tmpLen, i++) {
        if (file.block[i] == VD_NAN)
           file.block[i] = __getBlock();
        tmpLen = std::min(len, _magicBlock._blocks_size);
        vd.__write(file.block[i], ptr, tmpLen);
    }

    __saveFile(file);

    return file.size;
}

vd_size_t FileSystem::read(vd_size_t fd, char *ptr, vd_size_t len)
{
    fileData_t file = __getFileFromFD(fd);
    vd_size_t tmpLen = 0, readSize = 0;

    if (file.block[0] == VD_NAN) return 0;

    if (len + sizeof(fileData_t) > _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t))
        len = _magicBlock._blocks_size * MAX_NUMBER_BLOCK - sizeof(fileData_t);

    tmpLen = (len > _magicBlock._blocks_size - sizeof(fileData_t)) ? _magicBlock._blocks_size - sizeof(fileData_t) : len;
    vd.__read(file.block[0], ptr, tmpLen, sizeof(fileData_t));

    for (int i = 0; len > 0 && i < MAX_NUMBER_BLOCK; len -= tmpLen, i++) {
        if (file.block[i] == VD_NAN) {
            return -1;
        }
        ptr += tmpLen;
        readSize += tmpLen;
        tmpLen = (len > _magicBlock._blocks_size) ? _magicBlock._blocks_size : len;
        vd.__read(file.block[i], ptr, tmpLen);
    }

    return readSize;
}

#pragma endregion

#pragma region list

std::vector<fileStat_t> FileSystem::list(std::string path)
{
    dirData_t folder = getFolder(path);

    return list(folder);
}

std::vector<fileStat_t> FileSystem::list(dirData_t current)
{
    std::vector<fileStat_t> result;

    if (current.block == VD_NAN)
        current = __getFolder(MAIN_FOLDER_BLOCK);
    for (auto it: current.files)
        result.push_back(__stat(it));
    return result;
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
    } catch (std::out_of_range &_e) {
        std::ignore = _e;

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

    // fill freeblock
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

#pragma region get_save_remove_folder_file

void FileSystem::__remove(vd_size_t block)
{
    _magicBlock._nb_used_blocks--;
    _magicBlock._free_blocks.push_back(block);

    char ptr[5] = {0, 0, 0, 0, 0};
    vd.__write(block, ptr, 5);
}

void FileSystem::__remove(std::vector<vd_size_t> blocks)
{
    for (auto it: blocks)
        __remove(it);
}

void FileSystem::__saveFolder(dirData_t folder)
{
    vd_size_t sizef = sizeof(dirData_t) - sizeof(std::vector<vd_size_t>);

    vd.__write(folder.block, &folder, sizef);

    vd_size_t size = folder.files.size();
    vd.__write(folder.block, &size, sizeof(vd_size_t), sizef);
    sizef += sizeof(vd_size_t);

    vd.__write(folder.block, folder.files.data(), size * sizeof(vd_size_t), sizef);
}

dirData_t FileSystem::__getFolder(std::string name, dirData_t parent)
{
    if (name == ".") return parent;

    for (auto it : getFolderFromBlock(parent.files))
        if (!strcmp(it.name, name.c_str()))
            return it;
    throw std::runtime_error(name + ": folder not found");
}

dirData_t FileSystem::__getFolder(vd_size_t block)
{
    dirData_t folder;
    int sizef = sizeof(dirData_t) - sizeof(std::vector<vd_size_t>);
    vd_size_t size;
    vd_size_t offset = std::abs(sizef);

    vd.__read(block, &folder, offset);
    if (std::strcmp(folder.conf, FOLDER_CONF) != 0) {
        if (std::strcmp(folder.conf, FILE_CONF) == 0) return dirData_t();
        std::cerr << "error block:" << block << " conf= " << folder.conf  << std::endl;
        throw std::runtime_error("FileSystem::getFolderFromBlock(): the block dont refere to a dirData_t");
    }

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
        std::strcpy(stat.path, file.path);
        std::strcpy(stat.name, file.name);
        stat.size = file.size;
        stat.isFolder = false;

    } else if (strcmp(conf, FOLDER_CONF) == 0) {
        dirData_t folder = __getFolder(block);
        std::strcpy(stat.path, folder.path);
        std::strcpy(stat.name, folder.name);
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