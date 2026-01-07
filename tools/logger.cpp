#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cstdlib>
#include "txtlog.hpp"
#include "debug.hpp"

class CmdOptions
{
private:
    std::unordered_map<std::string, std::string> options;

    void parse(int argc, char *argv[])
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg(argv[i]);

            if (arg == "--help")
            {
                printHelp(argv[0]);
                std::exit(0);
            }

            if (arg.rfind("--", 0) == 0)
            {
                auto pos = arg.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = arg.substr(2, pos - 2);
                    std::string value = arg.substr(pos + 1);
                    options[key] = value;
                }
            }
        }
    }

public:
    CmdOptions(int argc, char *argv[])
    {
        parse(argc, argv);
    }

    std::string getString(const std::string &key,
                          const std::string &defaultValue) const
    {
        auto it = options.find(key);
        return (it != options.end()) ? it->second : defaultValue;
    }

    std::size_t getSizeT(const std::string &key,
                         std::size_t defaultValue) const
    {
        auto it = options.find(key);
        if (it == options.end())
        {
            return defaultValue;
        }

        std::size_t value{};
        std::istringstream iss(it->second);
        iss >> value;

        return iss.fail() ? defaultValue : value;
    }

    static void printHelp(const std::string &appName)
    {
        std::cout << R"(
_________________________________________________________________________

utils-logger is a command-line utility that captures all stdout output
from a target application via shell pipelines and writes it to managed
log files in a safe and efficient manner.

It operates transparently without requiring any modification to the
target application.

Key Features:
- Captures all console (stdout) output from the target application
- Configurable log file name and working directory
- Log file size limitation
- Automatic log rotation when size limit is reached
- Configurable number of .txt backup files
- Automatic compression of old log backups
- Configurable limit for compressed archive files
- Internal RAM buffering to reduce disk I/O
_________________________________________________________________________
)" << std::endl;

        std::cout << "Usage:\n"
                     "  stdbuf -oL <target_app> [target_app_options] | "
                  << appName << " [options]\n\n"
                                "Options:\n"
                                "  --workdir=<path>              Working directory for log files\n"
                                "                                Default: /var/log\n\n"
                                "  --filename=<name>             Base log file name\n"
                                "                                Default: log (without .txt)\n\n"
                                "  --max-size=<bytes>            Maximum log file size in bytes\n"
                                "                                Default: 20971520 (20 MB)\n\n"
                                "  --max-txt-backups=<count>     Number of .txt backup files\n"
                                "                                Default: 3\n\n"
                                "  --max-archive-files=<count>   Maximum archive backup files\n"
                                "                                Default: 10\n\n"
                                "  --buffer=<count>              Input buffer size\n"
                                "                                Default: 1024 bytes\n\n"
                                "  --help                        Show this help and exit\n";
    }
};

static void printConfig(const std::string &workDir,
                        const std::string &fileName,
                        std::size_t maxFileSize,
                        std::size_t maxTxtBackups,
                        std::size_t maxArchiveFiles,
                        std::size_t bsz)
{
    std::printf(
        "==== Logger Configuration ====\n"
        "Working directory   : %s\n"
        "Base file name      : %s\n"
        "Max file size       : %zu bytes\n"
        "Max .txt backups    : %zu\n"
        "Max archive files   : %zu\n"
        "Buffering           : %zu bytes\n"
        "===============================\n",
        workDir.c_str(),
        fileName.c_str(),
        maxFileSize,
        maxTxtBackups,
        maxArchiveFiles,
        bsz);
}

int main(int argc, char **argv)
{
    CmdOptions opts(argc, argv);

    const std::string workDir = opts.getString("workdir", "/var/log");
    const std::string fileName = opts.getString("filename", "log");
    const std::size_t maxFileSize = opts.getSizeT("max-size", 20971520UL);
    const std::size_t maxTxtBackups = opts.getSizeT("max-txt-backups", 3);
    const std::size_t maxArchiveFiles = opts.getSizeT("max-archive-files", 10);
    const std::size_t bsz = opts.getSizeT("buffer", 1024);

    printConfig(
        workDir,
        fileName,
        maxFileSize,
        maxTxtBackups,
        maxArchiveFiles,
        bsz);

    TXTLog log(
        workDir,
        fileName,
        maxFileSize,
        maxTxtBackups,
        maxArchiveFiles);

    std::string line;
    std::string toWrite;
    toWrite.reserve(bsz + 1024);

    while (std::getline(std::cin, line))
    {
        std::cout << line << std::endl;
        toWrite += line + "\n";
        if (toWrite.length() >= bsz)
        {
            log.write(toWrite);
            toWrite.clear();
        }
    }
    if (toWrite.empty() == false)
    {
        log.write(toWrite);
    }
    return 0;
}