#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <lzma.h>
#ifndef __DISABLE_MINIZIP
#include <minizip/zip.h>
#endif
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
    this->activeFilePath = workingDirectory + "/" + baseFileName + ".log";
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
    oss << this->workingDirectory << "/" << this->baseFileName << buffer << ".log";
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

bool TXTLog::extractXzToFile(const std::string &xzFile, const std::string &outputFile)
{
    std::ifstream input(xzFile, std::ios::binary);
    std::ofstream output(outputFile, std::ios::binary);

    if (!input || !output)
        return false;

    lzma_stream stream = LZMA_STREAM_INIT;
    if (lzma_stream_decoder(&stream, UINT64_MAX, 0) != LZMA_OK)
        return false;

    const std::size_t bufferSize = 4096;
    std::vector<uint8_t> inBuffer(bufferSize);
    std::vector<uint8_t> outBuffer(bufferSize);

    lzma_action action = LZMA_RUN;
    bool done = false;

    while (!done)
    {
        if (!input.eof())
        {
            input.read(reinterpret_cast<char *>(inBuffer.data()), bufferSize);
            stream.avail_in = input.gcount();
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

            std::size_t writeSize = bufferSize - stream.avail_out;
            output.write(reinterpret_cast<char *>(outBuffer.data()), writeSize);

            if (ret == LZMA_STREAM_END)
            {
                done = true;
                break;
            }
            else if (ret != LZMA_OK)
            {
                lzma_end(&stream);
                return false;
            }

        } while (stream.avail_out == 0);
    }

    lzma_end(&stream);
    return true;
}

#ifndef __DISABLE_MINIZIP
bool TXTLog::addFileToZip(void *zipHandle, const std::string &filePath, const std::string &entryName)
{
    zipFile zf = static_cast<zipFile>(zipHandle);

    std::ifstream input(filePath, std::ios::binary);
    if (!input)
        return false;

    zip_fileinfo zi = {};
    if (zipOpenNewFileInZip(zf,
                            entryName.c_str(),
                            &zi,
                            nullptr, 0,
                            nullptr, 0,
                            nullptr,
                            Z_DEFLATED,
                            Z_DEFAULT_COMPRESSION) != ZIP_OK)
        return false;

    const std::size_t bufferSize = 4096;
    char buffer[bufferSize];

    while (!input.eof())
    {
        input.read(buffer, bufferSize);
        std::size_t readSize = input.gcount();

        if (readSize > 0)
        {
            if (zipWriteInFileInZip(zf, buffer, readSize) < 0)
            {
                zipCloseFileInZip(zf);
                return false;
            }
        }
    }

    zipCloseFileInZip(zf);
    return true;
}

bool TXTLog::addFolderToZip(void *zipHandle, const std::string &basePath, const std::string &currentPath)
{
    zipFile zf = static_cast<zipFile>(zipHandle);

    DIR *dir = ::opendir(currentPath.c_str());
    if (!dir)
        return false;

    struct dirent *entry;

    while ((entry = ::readdir(dir)) != nullptr)
    {
        std::string name(entry->d_name);

        if (name == "." || name == "..")
            continue;

        std::string fullPath = currentPath + "/" + name;

        struct stat st;
        if (::stat(fullPath.c_str(), &st) != 0)
            continue;

        if (S_ISDIR(st.st_mode))
        {
            if (!addFolderToZip(zipHandle, basePath, fullPath))
            {
                ::closedir(dir);
                return false;
            }
        }
        else if (S_ISREG(st.st_mode))
        {
            std::string relativePath = fullPath.substr(basePath.length() + 1);

            if (!this->addFileToZip(zf, fullPath, relativePath))
            {
                ::closedir(dir);
                return false;
            }
        }
    }

    ::closedir(dir);
    return true;
}
#endif

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
        if (name.find(this->baseFileName) == 0 && name.find(".log") != std::string::npos)
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

#ifndef __DISABLE_MINIZIP
bool TXTLog::createZipSnapshot(const std::string &zipFilePath)
{
    std::lock_guard<std::mutex> lock(this->mutex);

    zipFile zf = zipOpen(zipFilePath.c_str(), APPEND_STATUS_CREATE);
    if (!zf)
        return false;

    std::vector<std::string> txtFiles = this->listBackupFiles();
    txtFiles.push_back(this->activeFilePath);

    for (const auto &file : txtFiles)
    {
        std::string entryName = file.substr(file.find_last_of("/\\") + 1);
        if (!this->addFileToZip(zf, file, entryName))
        {
            zipClose(zf, nullptr);
            return false;
        }
    }

    std::vector<std::string> archiveFiles = this->listArchiveFiles();

    for (const auto &xzFile : archiveFiles)
    {
        std::string tempTxt = xzFile.substr(0, xzFile.length() - 3) + ".log";

        if (!this->extractXzToFile(xzFile, tempTxt))
            continue;

        std::string entryName = tempTxt.substr(tempTxt.find_last_of("/\\") + 1);
        this->addFileToZip(zf, tempTxt, entryName);

        ::unlink(tempTxt.c_str());
    }

    zipClose(zf, nullptr);
    return true;
}

bool TXTLog::zipFolder(const std::string &folderPath, const std::string &zipFilePath)
{
    std::lock_guard<std::mutex> lock(this->mutex);

    zipFile zf = zipOpen64(zipFilePath.c_str(), APPEND_STATUS_CREATE);
    if (!zf)
        return false;

    bool result = this->addFolderToZip(zf, folderPath, folderPath);

    zipClose(zf, nullptr);
    return result;
}
#endif