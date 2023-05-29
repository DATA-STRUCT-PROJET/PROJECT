#include "prompt.hpp"
#include "fs.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

PromptCommand::PromptCommand(std::vector<std::string> _args)
    : m_args(_args)
{
    m_name = _args.at(0);
    m_args.erase(_args.begin());
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
    PromptCommand cmd = parse(line);

    try {
        for (auto &[_key, _fn] : m_prompMap)
            if (_key == cmd.getName())
                return (this->*_fn)(cmd);
    } catch (std::exception &_e) {
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
    m_prompMap["cd"] = &Prompt::fnCd; // done
    m_prompMap["ls"] = &Prompt::fnLs; // done
    m_prompMap["tree"] = &Prompt::fnTree;
    m_prompMap["cat"] = &Prompt::fnCat; // done
    m_prompMap["touch"] = &Prompt::fnTouch; // need test
    m_prompMap["rmdir"] = &Prompt::fnRmdir;
    m_prompMap["rm"] = &Prompt::fnRm;
    m_prompMap["mkdir"] = &Prompt::fnMkdir;
    m_prompMap["echo"] = &Prompt::fnEcho; // done
}

#pragma region Command function

PromptCommandResultEnum Prompt::fnCd(const PromptCommand &_cmd)
{
    fileStat_t stat;

    if (_cmd.getArgs().size() != 1)
        return PromptCommandResultEnum::ERROR;
    try {
        stat = m_fs.stat(m_cdir + _cmd.getArgs().front());
        if (stat.isFolder)
            m_cdir = stat.name;
        else
            m_os << "cd: " << _cmd.getArgs().front() << ": Not a directory" << std::endl;
    } catch (std::exception &_e) {
        std::ignore = _e;

        m_os << "cd: " << _cmd.getArgs().front() << ": No such file or directory" << std::endl;
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
            }
            std::cout << "fnLs path:" << _path << std::endl;
            if (narg > 1)
                m_os << _path << ":" << std::endl;
            dir = m_cdir + _path;
            for (auto &_file : m_fs.list(dir))
                m_os << _file.name << '\t';
            m_os << std::endl;
            if (narg > 1)
                m_os << std::endl;
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
            m_os << _path << ": is a directory" << std::endl;
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
        m_fs.create(m_cdir + '/' + _cmd.getArgs().front());
        // stat = m_fs.stat(_cmd.getArgs().front());
    } catch (std::exception &_e) {
        // std::ignore = _e;
        std::cerr << _e.what() << std::endl;
        // std::cerr << "command touch= " << m_cdir << _cmd.getArgs().front() << std::endl;
        // m_fs.create(m_cdir, _cmd.getArgs().front());
    }
    return PromptCommandResultEnum::SUCCESS;
}

PromptCommandResultEnum Prompt::fnRmdir(const PromptCommand &_cmd)
{
    // if (_cmd.getArgs().size() != 1)
    return PromptCommandResultEnum::ERROR;
    // return static_cast<PromptCommandResultEnum>(m_fs.removeDirecotry(_cmd.getArgs().front()));
}

PromptCommandResultEnum Prompt::fnRm(const PromptCommand &_cmd)
{
    // if (_cmd.getArgs().size() != 1)
    return PromptCommandResultEnum::ERROR;
    // return static_cast<PromptCommandResultEnum>(m_fs.remove(_cmd.getArgs().front()));
}

PromptCommandResultEnum Prompt::fnMkdir(const PromptCommand &_cmd)
{
    // if (_cmd.getArgs().size() != 1)
    return PromptCommandResultEnum::ERROR;
    // return static_cast<PromptCommandResultEnum>(m_fs.create(m_cdir, _cmd.getArgs().front()));
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
	if (_cmd.getArgs().size() < 2)
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
    for (size_t it = 0; it < _cmd.getArgs().size(); it++) {
        // m_fs.write(fd, reinterpret_cast<const void *>(_cmd.getArgs().at(it).data()), _cmd.getArgs().at(it).size());
        // not working?
        m_fs.write(fd, (void *)_cmd.getArgs().at(it).data(), _cmd.getArgs().at(it).size());
        // if (it != _cmd.getArgs().size())
        //     m_fs.write(fd, (void *)"\n", 1); // write dont concat!!!!!
    } 
    m_fs.close(fd);
    return PromptCommandResultEnum::SUCCESS;
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
    std::transform(args.at(0).begin(), args.at(0).end(), args.at(0).begin(), [] (const char c) {
        return std::tolower(c);
    });
    return PromptCommand(args);
}