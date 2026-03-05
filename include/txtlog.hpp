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

    /**
     * @brief Extract archive from the given files.
     *
     * @param xzFile The archive file path.
     * @param outputFile The txt file name as output file name.
     *
     * @return true if success.
     * @return false on fail.
     */
    bool extractXzToFile(const std::string &xzFile, const std::string &outputFile);

#ifndef __DISABLE_MINIZIP
    /**
     * @brief Adds a single file from the filesystem into an opened ZIP archive.
     *
     * This method reads the content of a given file in binary mode and writes it
     * as a new entry inside an already opened ZIP archive using the minizip API.
     *
     * The ZIP archive handle must be valid and previously created using
     * zipOpen() or zipOpen64(). The function will:
     *
     * 1. Open the source file specified by @p filePath.
     * 2. Create a new file entry inside the ZIP archive with the name @p entryName.
     * 3. Stream the file content into the ZIP archive using deflate compression.
     * 4. Close the ZIP entry properly.
     *
     * Compression method:
     * - Z_DEFLATED
     * - Z_DEFAULT_COMPRESSION
     *
     * The method performs streaming writes using a fixed-size buffer to avoid
     * loading the entire file into memory.
     *
     * @param zipHandle Opaque pointer to an opened ZIP archive (zipFile).
     * @param filePath Absolute or relative filesystem path of the source file to be added into the archive.
     * @param entryName The name of the file entry inside the ZIP archive.
     *
     * @return true if success.
     * @return false on fail.
     */
    bool addFileToZip(void *zipHandle, const std::string &filePath, const std::string &entryName);

    /**
     * @brief Recursively traverses a directory and adds its contents to an open ZIP archive.
     *
     * This internal helper method performs a depth-first traversal of a directory
     * structure starting from @p currentPath and adds all regular files into the
     * specified ZIP archive.
     *
     * The method preserves the relative directory structure inside the ZIP file.
     * The relative path of each entry is computed using @p basePath as the root.
     *
     * Behavior:
     * - Iterates over all directory entries in @p currentPath.
     * - Skips "." and "..".
     * - If an entry is a subdirectory, the function calls itself recursively.
     * - If an entry is a regular file (S_ISREG), it is added to the ZIP archive
     *   using addFileToZip().
     * - Symbolic links and non-regular files are ignored.
     *
     * Path Handling:
     * - basePath represents the root directory of the archive.
     * - currentPath represents the directory currently being traversed.
     * - The ZIP entry name is computed as:
     *
     *       relativePath = fullPath.substr(basePath.length() + 1)
     *
     *   ensuring the archive structure mirrors the folder hierarchy.
     *
     * @param zipHandle Opaque pointer to an opened ZIP archive (zipFile).
     * @param basePath Root directory used to compute relative paths inside the archive.
     * @param currentPath The directory currently being scanned. During recursion, this value changes to represent subdirectories.
     *
     * @retval true when all files and subdirectories were successfully added to the archive.
     *
     * @retval false when Traversal failed due to:
     *         - Inability to open a directory.
     *         - Failure while adding a file into the ZIP archive.
     *         - Filesystem access error.
     */
    bool addFolderToZip(void *zipHandle, const std::string &basePath, const std::string &currentPath);
#endif

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
           const std::string &baseFileName = "log.log",
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

#ifndef __DISABLE_MINIZIP
    /**
     * @brief Creates a ZIP snapshot containing all currently stored log files.
     *
     * This method generates a consolidated ZIP archive that contains:
     *
     * 1. All active and rotated plain-text log files (.log) currently present
     *    in the working directory.
     * 2. All archived log files (.xz), which are first decompressed into
     *    temporary .log files before being added into the ZIP archive.
     *
     * Workflow:
     * - Acquire internal mutex to ensure thread safety during snapshot creation.
     * - Enumerate all available .log files (including the active log file).
     * - Add each .log file directly into the ZIP archive.
     * - Enumerate all .xz archive files.
     * - For each .xz file:
     *      a. Decompress into a temporary .log file.
     *      b. Add the temporary file into the ZIP archive.
     *      c. Remove the temporary extracted file immediately after insertion.
     * - Close the ZIP archive.
     *
     * The resulting ZIP archive will contain only .log entries,
     * regardless of whether their source was a plain log file or an .xz archive.
     *
     * @param zipFilePath Absolute or relative path where the resulting ZIP archive will be created.
     *
     * @return true if success.
     * @return false on fail.
     */
    bool createZipSnapshot(const std::string &zipFilePath);

    /**
     * @brief Compresses an entire directory into a ZIP archive.
     *
     * Recursively traverses the specified directory and adds all regular files
     * into a newly created ZIP archive while preserving relative paths.
     *
     * @param folderPath   Path to the directory to compress.
     * @param zipFilePath  Output ZIP file path.
     *
     * @retval true  Folder successfully archived.
     * @retval false Failure occurred during traversal or compression.
     */
    bool zipFolder(const std::string &folderPath, const std::string &zipFilePath);
#endif
};

#endif