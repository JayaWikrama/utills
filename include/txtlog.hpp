/*
 * $Id: txtlog.hpp, v 1.0.0 2025/12/26 10:00:00 Jaya Wikrama Exp $
 *
 * Copyright (c) 2024 Jaya Wikrama
 * jayawikrama89@gmail.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file txtlog.hpp
 * @brief Text file logger with size-based rotation, backup, and archiving.
 *
 * This file defines the TXTLog class, which provides functionality to write
 * log data into a text file with automatic file rotation based on maximum
 * file size. When the file size limit is reached, the active log file is
 * renamed into a timestamped backup file.
 *
 * The class maintains:
 * - A fixed number of plain text backup files.
 * - Older backup files are automatically archived into xz files,
 *   with a fixed maximum number of archive files.
 *
 * @version 1.0.0
 * @date 2025-12-26
 * @author Jaya Wikrama
 */

#ifndef TXT_LOG_HPP
#define TXT_LOG_HPP

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <mutex>

/**
 * @class TXTLog
 * @brief Size-based rotating text file logger with backup and archive support.
 *
 * TXTLog writes textual data to a log file. Once the file size exceeds the
 * configured maximum size, the file is rotated and renamed using a timestamp
 * format:
 *
 *   <base_filename>YYYY.MM.DD.HH.MM.SS.txt
 *
 * The logger keeps a limited number of plain text backup files. Older backups
 * are compressed into xz archives with a fixed maximum number of archive files.
 *
 * This class is designed for Linux environments and can be extended to use
 * POSIX file APIs or standard C++ streams.
 */
class TXTLog
{
private:
    int fileDescriptor;

    std::string workingDirectory;
    std::string baseFileName;
    std::string activeFilePath;

    std::size_t maxFileSize;
    std::size_t maxTxtBackups;
    std::size_t maxArchiveFiles;

    mutable std::mutex mutex;

    /* ================= File Handling ================= */

    /**
     * @brief Open the active log file.
     *
     * @return true if successful, false otherwise.
     */
    bool openActiveFile();

    /**
     * @brief Rotate the log file if size limit is exceeded.
     */
    void rotateIfNeeded();

    /**
     * @brief Check whether file rotation is required.
     *
     * @param incomingDataSize Size of data to be written.
     * @return true if rotation is required, false otherwise.
     */
    bool isRotationRequired(std::size_t incomingDataSize) const;

    /* ================= Backup Handling ================= */

    /**
     * @brief Generate a timestamped backup file name.
     *
     * @return Backup file name with timestamp.
     */
    std::string generateTimestampedBackupName() const;

    /**
     * @brief Create a .txt backup from the active log file.
     */
    void createTxtBackup();

    /**
     * @brief Maintain the number of plain text backup files.
     *
     * Excess backup files will be archived or removed as needed.
     */
    void maintainTxtBackups();

    /* ================= Archive Handling ================= */

    /**
     * @brief Generate archive file name.
     *
     * @param fileName The file name.
     * @param ext Archive file extention.
     *
     * @return Archive file name.
     */
    std::string generateArchiveName(const std::string &fileName = "", const std::string ext = ".xz") const;

    /**
     * @brief Create archive from the given files.
     *
     * @param files List of files to be archived.
     * @return true if archive creation succeeds, false otherwise.
     */
    bool createArchive(const std::vector<std::string> &files);

    /**
     * @brief Maintain the number of archived backup files.
     *
     * Excess archived backup files will removed as needed.
     */
    void maintainArchivedBackups();

    /* ================= Utility ================= */

    /**
     * @brief Get the current size of the active log file.
     *
     * @return File size in bytes.
     */
    std::uintmax_t getCurrentFileSize() const;

    /**
     * @brief List all existing backup files.
     *
     * @return Vector of backup file paths.
     */
    std::vector<std::string> listBackupFiles() const;

    /**
     * @brief List all existing archive files.
     *
     * @return Vector of archive file paths.
     */
    std::vector<std::string> listArchiveFiles() const;

    /**
     * @brief Remove specified files from filesystem.
     *
     * @param files List of file paths to remove.
     */
    void removeFiles(const std::vector<std::string> &files);

public:
    /**
     * @brief Construct a TXTLog instance.
     *
     * @param workingDirectory   Path of the active log file.
     * @param baseFileName       Base name of the active log file.
     * @param maxFileSize        Maximum allowed size of a log file in bytes.
     * @param maxTxtBackups      Number of .txt backup files to keep.
     * @param maxArchiveFiles    Maximum number of backup archive files.
     */
    TXTLog(const std::string &workingDirectory = ".",
           const std::string &baseFileName = "log.txt",
           std::size_t maxFileSize = 20971520,
           std::size_t maxTxtBackups = 3,
           std::size_t maxArchiveFiles = 10);

    /**
     * @brief Destructor.
     *
     * Ensures that any open file handles are properly closed.
     */
    ~TXTLog();

    /**
     * @brief Write data to the log file.
     *
     * If writing the data causes the file to exceed the maximum size,
     * file rotation will be performed automatically.
     *
     * @param data Text data to be written.
     * @return true if the write operation succeeds, false otherwise.
     */
    bool write(const std::string &data);

    /**
     * @brief Flush buffered data to disk.
     */
    void flush();

    /**
     * @brief Close the active log file.
     */
    void close();

    /**
     * @brief Set a new maximum file size.
     *
     * @param maxFileSize New maximum size in bytes.
     */
    void setMaxFileSize(std::size_t maxFileSize);

    /**
     * @brief Get the current maximum file size.
     *
     * @return Maximum file size in bytes.
     */
    std::size_t getMaxFileSize() const;
};

#endif