#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <lzma.h>
#include <fstream>

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <sstream>
#include <mutex>

#include "debug.hpp"
#include "txtlog.hpp"

/* ================= Constructor / Destructor ================= */

TXTLog::TXTLog(const std::string &workingDirectory,
               const std::string &baseFileName,
               std::size_t maxFileSize,
               std::size_t maxTxtBackups,
               std::size_t maxArchiveFiles) : fileDescriptor(-1),
                                              workingDirectory(workingDirectory),
                                              baseFileName(baseFileName),
                                              activeFilePath(),
                                              maxFileSize(maxFileSize),
                                              maxTxtBackups(maxTxtBackups),
                                              maxArchiveFiles(maxArchiveFiles),
                                              mutex()
{
    this->activeFilePath = workingDirectory + "/" + baseFileName + ".txt";
    this->openActiveFile();
    this->rotateIfNeeded();
}

TXTLog::~TXTLog()
{
    this->close();
}

/* ================= Public API ================= */

bool TXTLog::write(const std::string &data)
{
    std::lock_guard<std::mutex> lock(this->mutex);

    if (this->fileDescriptor <= 0)
    {
        if (!this->openActiveFile())
        {
            return false;
        }
    }

    this->rotateIfNeeded();

    ssize_t written = ::write(this->fileDescriptor, data.c_str(), data.size());

    if (written != static_cast<ssize_t>(data.size()))
    {
        Debug::error(__FILE__, __LINE__, __func__, "failed\n");
        return false;
    }
    return true;
}

void TXTLog::flush()
{
    std::lock_guard<std::mutex> lock(this->mutex);

    if (this->fileDescriptor > 0)
    {
        ::fsync(this->fileDescriptor);
    }
}

void TXTLog::close()
{
    std::lock_guard<std::mutex> lock(this->mutex);

    if (this->fileDescriptor > 0)
    {
        ::close(this->fileDescriptor);
        this->fileDescriptor = -1;
    }
}

void TXTLog::setMaxFileSize(std::size_t maxFileSize)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    this->maxFileSize = maxFileSize;
}

std::size_t TXTLog::getMaxFileSize() const
{
    return this->maxFileSize;
}

/* ================= File Handling ================= */

bool TXTLog::openActiveFile()
{
    this->fileDescriptor = ::open(this->activeFilePath.c_str(),
                                  O_CREAT | O_APPEND | O_WRONLY,
                                  0644);

    Debug::info(__FILE__, __LINE__, __func__, "%s\n", this->activeFilePath.c_str());
    if (this->fileDescriptor < 0)
    {
        Debug::error(__FILE__, __LINE__, __func__, "failed\n");
        return false;
    }
    if (this->fileDescriptor == 0)
    {
        ::close(this->fileDescriptor);
        this->fileDescriptor = -1;
        Debug::error(__FILE__, __LINE__, __func__, "failed\n");
        return false;
    }

    return true;
}

void TXTLog::rotateIfNeeded()
{
    if (this->fileDescriptor <= 0)
    {
        Debug::error(__FILE__, __LINE__, __func__, "failed: empty handle\n");
        return;
    }

    if (!this->isRotationRequired(0))
    {
        return;
    }

    ::close(this->fileDescriptor);
    this->fileDescriptor = -1;

    this->createTxtBackup();
    this->maintainTxtBackups();
    this->maintainArchivedBackups();

    this->openActiveFile();
}

bool TXTLog::isRotationRequired(std::size_t incomingDataSize) const
{
    std::uintmax_t currentSize = this->getCurrentFileSize();
    return (currentSize + incomingDataSize) >= this->maxFileSize;
}

/* ================= Backup Handling ================= */

std::string TXTLog::generateTimestampedBackupName() const
{
    std::time_t now = std::time(nullptr);
    std::tm tmNow;
    localtime_r(&now, &tmNow);

    char buffer[64];
    std::strftime(buffer, sizeof(buffer),
                  "_%Y%m%d.%H%M%S", &tmNow);

    std::ostringstream oss;
    oss << this->workingDirectory << "/" << this->baseFileName << buffer << ".txt";
    return oss.str();
}

void TXTLog::createTxtBackup()
{
    std::string backupName = this->generateTimestampedBackupName();
    ::rename(this->activeFilePath.c_str(), backupName.c_str());
}

void TXTLog::maintainTxtBackups()
{
    std::vector<std::string> backups = this->listBackupFiles();

    if (backups.size() <= this->maxTxtBackups)
    {
        return;
    }

    /* collect archivable backups */
    std::sort(backups.begin(), backups.end());

    std::vector<std::string> toArchive(
        backups.begin(),
        backups.end() - this->maxTxtBackups);

    /* archive file and delete archived files */
    this->createArchive(toArchive);
    this->removeFiles(toArchive);

    Debug::info(__FILE__, __LINE__, __func__, "success\n");
}

/* ================= Archive Handling ================= */

std::string TXTLog::generateArchiveName(const std::string &fileName, const std::string ext) const
{
    if (fileName.empty())
        return this->workingDirectory + "/archive_" + this->baseFileName + ext;

    size_t lastSlash = fileName.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        return this->workingDirectory + "/archive_" + fileName.substr(lastSlash + 1, fileName.length() - (lastSlash + 5)) + ext;

    return this->workingDirectory + "/archive_" + fileName.substr(0, fileName.length() - 4) + ext;
}

bool TXTLog::createArchive(const std::vector<std::string> &files)
{
    if (files.empty())
    {
        return true;
    }

    for (const std::string &txtFile : files)
    {
        std::string xzFile = generateArchiveName(txtFile, ".xz");
        std::ifstream inputFile(txtFile, std::ios::binary);
        std::ofstream outputFile(xzFile, std::ios::binary);

        lzma_stream stream = LZMA_STREAM_INIT;
        if (lzma_easy_encoder(&stream, 6, LZMA_CHECK_CRC64) != LZMA_OK)
            return false;

        const std::size_t bufferSize = 4096;
        std::vector<unsigned char> inBuffer(bufferSize);
        std::vector<unsigned char> outBuffer(bufferSize);
        lzma_action action = LZMA_RUN;

        bool isRun = true;

        while (isRun)
        {
            if (!inputFile.eof())
            {
                inputFile.read(reinterpret_cast<char *>(inBuffer.data()), bufferSize);
                stream.avail_in = inputFile.gcount();
                stream.next_in = inBuffer.data();
            }
            else
            {
                stream.avail_in = 0;
                action = LZMA_FINISH;
            }

            do
            {
                stream.avail_out = bufferSize;
                stream.next_out = outBuffer.data();

                lzma_ret ret = lzma_code(&stream, action);

                size_t writeSize = bufferSize - stream.avail_out;
                outputFile.write(reinterpret_cast<char *>(outBuffer.data()), writeSize);

                if (ret == LZMA_STREAM_END)
                {
                    lzma_end(&stream);
                    Debug::info(__FILE__, __LINE__, __func__, "file %s archived as %s\n", txtFile.c_str(), xzFile.c_str());
                    isRun = false;
                    break;
                }
                else if (ret != LZMA_OK)
                {
                    lzma_end(&stream);
                    Debug::info(__FILE__, __LINE__, __func__, "failed to archive file %s\n", txtFile.c_str());
                    isRun = false;
                    break;
                }
            } while (stream.avail_out == 0);
        }
    }
    return true;
}

void TXTLog::maintainArchivedBackups()
{
    std::vector<std::string> backups = this->listArchiveFiles();

    if (backups.size() <= this->maxArchiveFiles)
    {
        return;
    }

    /* collect old backups */
    std::sort(backups.begin(), backups.end());

    std::vector<std::string> toRemove(
        backups.begin(),
        backups.end() - this->maxArchiveFiles);

    /* delete old archive files */
    this->removeFiles(toRemove);

    Debug::info(__FILE__, __LINE__, __func__, "success\n");
}

/* ================= Utility ================= */

std::uintmax_t TXTLog::getCurrentFileSize() const
{
    struct stat st;
    if (::stat(this->activeFilePath.c_str(), &st) == 0)
    {
        return static_cast<std::uintmax_t>(st.st_size);
    }
    return 0;
}

std::vector<std::string> TXTLog::listBackupFiles() const
{
    std::vector<std::string> result;

    DIR *dp = ::opendir(this->workingDirectory.c_str());
    if (!dp)
    {
        return result;
    }

    struct dirent *entry;
    while ((entry = ::readdir(dp)) != nullptr)
    {
        std::string name(entry->d_name);
        if (name.find(this->baseFileName) == 0 && name.find(".txt") != std::string::npos)
        {
            result.push_back(this->workingDirectory + "/" + name);
        }
    }

    ::closedir(dp);
    return result;
}

std::vector<std::string> TXTLog::listArchiveFiles() const
{
    std::vector<std::string> result;

    DIR *dp = ::opendir(this->workingDirectory.c_str());
    if (!dp)
    {
        return result;
    }

    struct dirent *entry;
    std::string baseArchiveName = "archive_" + this->baseFileName;
    while ((entry = ::readdir(dp)) != nullptr)
    {
        std::string name(entry->d_name);
        if (name.find(baseArchiveName) == 0 && name.find(".xz") != std::string::npos)
        {
            result.push_back(this->workingDirectory + "/" + name);
        }
    }

    ::closedir(dp);
    return result;
}

void TXTLog::removeFiles(const std::vector<std::string> &files)
{
    for (const auto &file : files)
    {
        ::unlink(file.c_str());
    }
}
