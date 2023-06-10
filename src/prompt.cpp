#include "prompt.hpp"
#include "fs.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <cstring>

PromptCommand::PromptCommand(std::vector<std::string> _args)
    : m_args(_args)
{
    m_name = m_args.at(0);
    m_args.erase(m_args.begin());
}

const std::string &PromptCommand::getName() const
{
    return m_name;
}

const std::vector<std::string> &PromptCommand::getArgs() const
{
    return m_args;
}

Prompt::Prompt(std::ostream &os, FileSystem &_fs)
    : m_fs(_fs), m_os(os), m_cdir("/")
{
	generateMap();
}

std::string Prompt::GetPromptString() const
{
    return "\x1b[31m" + m_cdir + "\033[0m > ";
}

PromptCommandResultEnum Prompt::process(const std::string &line)
{
    try {
        PromptCommand cmd = parse(line);
        for (auto &[_key, _fn] : m_prompMap)
            if (_key == cmd.getName())
                return (this->*_fn)(cmd);
    } catch (std::exception &_e) {
        std::ignore = _e;

        return PromptCommandResultEnum::FAILURE;
    }
    return PromptCommandResultEnum::ERROR;
}

void Prompt::generateMap()
{
    static bool isGenerated = false;

    if (isGenerated)
        return;
    isGenerated = true;
    m_prompMap["cd"] = &Prompt::fnCd; // need some clarification on the API behavior
    m_prompMap["ls"] = &Prompt::fnLs;
    m_prompMap["tree"] = &Prompt::fnTree; // need an implementation
    m_prompMap["cat"] = &Prompt::fnCat;
    m_prompMap["touch"] = &Prompt::fnTouch;
    m_prompMap["rmdir"] = &Prompt::fnRmdir;
    m_prompMap["rm"] = &Prompt::fnRm;
    m_prompMap["mkdir"] = &Prompt::fnMkdir;
    m_prompMap["echo"] = &Prompt::fnEcho;
    m_prompMap["save"] = &Prompt::fnSave; // need testing
}

#pragma region Command function

PromptCommandResultEnum Prompt::fnCd(const PromptCommand &_cmd)
{
    fileStat_t stat;
    std::string folder = m_cdir;

    if (_cmd.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    try {
        if (_cmd.getArgs().front() == "..") {
            if (m_cdir == "/") {
                m_os << "cd: " << _cmd.getArgs().front() << ": No such file or directory" << std::endl;
                return PromptCommandResultEnum::FAILURE;
            }
            folder = getPath(m_cdir);
            if (folder.empty())
                folder = "/";
        } else {
            if (folder.back() != '/' && _cmd.getArgs().front().front() != '/')
                folder += "/";
            folder += _cmd.getArgs().front();
        }
        stat = m_fs.stat(folder);
        if (stat.isFolder) {
            m_cdir.clear();
            if (strcmp(stat.path, "/")) {
                if (!strlen(stat.path)) m_cdir = "/";
                for (size_t it = 0; stat.path[it]; it++) m_cdir += stat.path[it];
                if (strlen(stat.path)) m_cdir += "/";
                for (size_t it = 0; stat.name[it]; it++) m_cdir += stat.name[it];
            } else {
                m_cdir = "/";
            }
        } else {
            m_os << "cd: " << _cmd.getArgs().front() << ": Not a directory" << std::endl;
        }
    } catch (std::exception &_e) {
        std::ignore = _e;

        m_os << "cd: " << _cmd.getArgs().front() << ": No such file or directory" << std::endl;
        return PromptCommandResultEnum::FAILURE;
    }
    return static_cast<PromptCommandResultEnum>(stat.isFolder);
}

PromptCommandResultEnum Prompt::fnLs(const PromptCommand &_cmd)
{
    std::string dir{};
    size_t narg = _cmd.getArgs().size();
    fileStat_t stat;
    PromptCommandResultEnum ret = PromptCommandResultEnum::SUCCESS;

    if (narg > 0) {
        for (const auto &_path : _cmd.getArgs()) {
            try {
                stat = m_fs.stat(_path);
            } catch (std::exception &_e) {
                std::ignore = _e;

                m_os << "ls: cannot access " << _path << ": No such file or directory" << std::endl;
                ret = PromptCommandResultEnum::FAILURE;
                continue;
            } try {
            if (narg > 1)
                m_os << _path << ":" << std::endl;
            dir = m_cdir + _path;
            for (auto &_file : m_fs.list(dir))
                m_os << _file.name << '\t';
            m_os << std::endl;
            if (narg > 1)
                m_os << std::endl;
            } catch (std::exception &_e) {
                std::ignore = _e;

                m_os << "ls: cannot access " << _path << ": No such file or directory" << std::endl;
                ret = PromptCommandResultEnum::FAILURE;
            }
        }
    } else {
        try {
            for (const auto &_file : m_fs.list(m_cdir))
                m_os << _file.name << '\t';
            m_os << std::endl;
        } catch (std::exception &_e) {
            std::cerr << _e.what() << std::endl;
            ret = PromptCommandResultEnum::FAILURE;
        }
    }
    return ret;
}

PromptCommandResultEnum Prompt::fnTree(const PromptCommand &_cmd)
{
    return PromptCommandResultEnum::ERROR;
}

PromptCommandResultEnum Prompt::fnCat(const PromptCommand &_cmd)
{
    vd_size_t fd = 0;
    vd_size_t readlen = 0;
    fileStat_t stat;
    std::string data{};
    PromptCommandResultEnum ret = PromptCommandResultEnum::SUCCESS;

    if (_cmd.getArgs().empty())
        return PromptCommandResultEnum::FAILURE;
    if (_cmd.getArgs().front() == "--help") {
        m_os << "Usage: cat [FILE]..." << std::endl;
        m_os << "Concatenate FILE(s) to standard output." << std::endl;
        return PromptCommandResultEnum::SUCCESS;
    }
    for (const auto &_path : _cmd.getArgs()) {
        try {
            stat = m_fs.stat(_path);
        } catch (std::exception &_e) {
            std::ignore = _e;

            m_os << "cat: " << _path << ": No such file or directory" << std::endl;
            ret = PromptCommandResultEnum::FAILURE;
            continue;
        }
        if (stat.isFolder) {
            ret = PromptCommandResultEnum::FAILURE;
            m_os << "cat: " << _path << ": is a directory" << std::endl;
            continue;
        }
        fd = m_fs.open(_path); // check for fd == -1
        data.resize(stat.size);
        readlen = m_fs.read(fd, data.data(), stat.size);
	    m_os << data;
        m_fs.close(fd);
    	if (!readlen)
            ret = PromptCommandResultEnum::FAILURE;
    }
    return ret;
}

PromptCommandResultEnum Prompt::fnTouch(const PromptCommand &_cmd)
{
    fileStat_t stat;

    if (_cmd.getArgs().empty())
        return PromptCommandResultEnum::FAILURE;
    try {
        stat = m_fs.stat(_cmd.getArgs().front());
    } catch (std::exception &_e) {
        std::ignore = _e;
        m_fs.create(m_cdir + "/" + _cmd.getArgs().front());
    }
    return PromptCommandResultEnum::SUCCESS;
}

PromptCommandResultEnum Prompt::fnRmdir(const PromptCommand &_cmd)
{
    if (_cmd.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;

    std::string dirname = _cmd.getArgs().front();
    fileStat_t stat;
    try {
        stat = m_fs.stat(dirname);
    } catch (std::exception &_e) {
        std::ignore = _e;

        m_os << "rmdir: " << dirname << ": No such file or directory" << std::endl;
        return PromptCommandResultEnum::FAILURE;
    }

    if (!stat.isFolder) {
        m_os << "rmdir: " << dirname << ": is a file" << std::endl;
        return PromptCommandResultEnum::FAILURE;
    }

    try {
        if (m_fs.removeDirectory(dirname)) {
            return PromptCommandResultEnum::SUCCESS;
        } else {
            return PromptCommandResultEnum::FAILURE;
        }
    }
    catch (std::exception &_e){
        std::cerr << _e.what() << std::endl;
        return PromptCommandResultEnum::ERROR;
    }
}

PromptCommandResultEnum Prompt::fnRm(const PromptCommand &_cmd)
{
    if (_cmd.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;

    std::string filename = _cmd.getArgs().front();
    fileStat_t stat;
    try {
        stat = m_fs.stat(filename);
    } catch (std::exception &_e) {
        std::ignore = _e;

        m_os << "rm: " << filename << ": No such file or directory" << std::endl;
        return PromptCommandResultEnum::FAILURE;
    }

    if (stat.isFolder) {
        m_os << "rm: " << filename << ": is a directory" << std::endl;
        return PromptCommandResultEnum::FAILURE;
    }
    
    try {
        if (m_fs.remove(filename)) {
            return PromptCommandResultEnum::SUCCESS;
        } else {
            return PromptCommandResultEnum::FAILURE;
        }
    }
    catch (std::exception &_e){
        std::cerr << _e.what() << std::endl;
        return PromptCommandResultEnum::ERROR;
    }
}

PromptCommandResultEnum Prompt::fnMkdir(const PromptCommand &_cmd)
{
    std::string folder = m_cdir;

    if (_cmd.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    try {
        if (folder.back() != '/' && _cmd.getArgs().front().front() != '/')
            folder += "/";
        return static_cast<PromptCommandResultEnum>(m_fs.createFolder(folder + _cmd.getArgs().front()));
    } catch (std::exception &_e) {
        std::ignore = _e;
        std::cerr << "command mkdir= " << m_cdir << _cmd.getArgs().front() << std::endl;
        return PromptCommandResultEnum::ERROR;
    }
}

PromptCommandResultEnum Prompt::fnEcho(const PromptCommand &_cmd)
{
	std::string path{};
    vd_size_t fd;
    fileStat_t stat;

    if (_cmd.getArgs().front() == "-h" || _cmd.getArgs().front() == "--help") {
        m_os << "Usage: echo FILE [DATA]..." << std::endl;
        m_os << "Concatenate DATA(s) to FILE." << std::endl;
        return PromptCommandResultEnum::SUCCESS;
    }
	if (_cmd.getArgs().size() != 2)
        return PromptCommandResultEnum::FAILURE;
    path = _cmd.getArgs().front();
    try {
        stat = m_fs.stat(path);
    } catch (std::exception &_e) {
        std::ignore = _e;

        m_os << "echo: " << path << ": No such file or directory" << std::endl;
        return PromptCommandResultEnum::FAILURE;
    }
    if (stat.isFolder) {
        m_os << path << ": is a directory" << std::endl;
        return PromptCommandResultEnum::FAILURE;
    }
    fd = m_fs.open(path);
    m_fs.write(fd, (void *)_cmd.getArgs().front().data(), _cmd.getArgs().front().size());
    m_fs.close(fd);
    return PromptCommandResultEnum::SUCCESS;
}

PromptCommandResultEnum Prompt::fnSave(const PromptCommand &_cmd)
{
    if (!_cmd.getArgs().empty() && (_cmd.getArgs().front() == "-h" || _cmd.getArgs().front() == "--help")) {
        m_os << "Usage: save FILE" << std::endl;
        m_os << "Save the actual virtual disk into the FILE." << std::endl;
        return PromptCommandResultEnum::SUCCESS;
    }
    if (_cmd.getArgs().size() == 1) {
        m_fs.save(_cmd.getArgs().front());
        std::cout << "The FileSystem as been saved under the file name: " << _cmd.getArgs().front() << std::endl;
        return PromptCommandResultEnum::SUCCESS;
    }
    return PromptCommandResultEnum::FAILURE;
}

#pragma endregion

PromptCommand Prompt::parse(const std::string &line)
{
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> args;

    // handle multi spacing and tabulation between exec and args
    // handle "" for args
    while (std::getline(ss, token, ' '))
        args.push_back(token);
    if (args.size() == 0 ){
        throw std::runtime_error("No input provided");
    }
    std::transform(args.at(0).begin(), args.at(0).end(), args.at(0).begin(), [] (const char c) {
        return std::tolower(c);
    });
    return PromptCommand(args);
}