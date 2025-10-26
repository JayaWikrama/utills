/*
 * $Id: debug.cpp, v 1.2.0 2025/09/20 21:28:23 Jaya Wikrama Exp $
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
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include "debug.hpp"
#include "queue.hpp"

typedef Queue<char *> QueueHistory;

Debug::Debug(size_t maxLineLogs) : maxLineLogs(maxLineLogs)
{
    if (this->maxLineLogs)
    {
        QueueHistory *q = new QueueHistory;
        this->history = (QueuePtr)q;
    }
    else
    {
        this->history = nullptr;
    }
}

Debug::~Debug()
{
    clearLogHistory();
    if (this->history)
    {
        QueueHistory *q = (QueueHistory *)this->history;
        delete q;
    }
}

size_t Debug::getMaxLineLogs()
{
    return this->maxLineLogs;
}

size_t Debug::getHistoriesNumber()
{
    if (this->history)
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        QueueHistory *q = (QueueHistory *)this->history;
        return q->size();
    }
    return 0;
}

#ifdef CXXSTD_17
std::string_view Debug::logTypeToString(Debug::LogType_t type)
#else
std::string Debug::logTypeToString(Debug::LogType_t type)
#endif
{
#ifdef CXXSTD_17
    static constexpr std::string_view logTypeStr[] = {"I", "W", "E", "C"};
#else
    static std::string logTypeStr[] = {"I", "W", "E", "C"};
#endif
    return logTypeStr[static_cast<int>(type)];
}

#ifdef CXXSTD_17
std::string_view Debug::extractFileName(const char *fileName)
#else
std::string Debug::extractFileName(const char *fileName)
#endif
{
    const char *slash = strrchr(fileName, '/');
    if (!slash)
        slash = strrchr(fileName, '\\');
    return slash ? slash + 1 : fileName;
}

#ifdef CXXSTD_17
std::string_view Debug::extractFunctionName(const char *functionName)
#else
std::string Debug::extractFunctionName(const char *functionName)
#endif
{
#ifdef CXXSTD_17
    std::string_view fn(functionName);
#else
    std::string fn(functionName);
#endif
    size_t start = fn.find_last_of(' ');

#ifdef CXXSTD_17
    start = (start == std::string_view::npos) ? 0 : start + 1;
#else
    start = (start == std::string::npos) ? 0 : start + 1;
#endif

    size_t end = fn.find('(', start);

#ifdef CXXSTD_17
    return fn.substr(start, end == std::string_view::npos
                                ? fn.size() - start
                                : end - start);
#else
    return fn.substr(start, end == std::string::npos
                                ? fn.size() - start
                                : end - start);
#endif
}

std::string Debug::generate(Debug::LogType_t type,
                            const char *functionName,
                            const char *format,
                            va_list args)
{
    return Debug::generate(type, nullptr, 0, functionName, format, args);
}

void Debug::log(LogType_t type, const char *functionName, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::string logEntry = this->generate(type, functionName, format, args);
    va_end(args);

    std::cout << logEntry;

    if (this->history)
    {
        char *line = new char[logEntry.length() + 1];
        if (line)
        {
            strcpy(line, logEntry.c_str());
            std::lock_guard<std::mutex> lock(this->mutex);
            QueueHistory *q = (QueueHistory *)this->history;
            q->enqueue(line);
            if (q->size() > maxLineLogs)
            {
                line = q->dequeue();
                delete[] line;
            }
        }
    }
}

std::string Debug::getLogHistory() const
{
    std::ostringstream oss;
    if (this->history)
    {
        std::lock_guard<std::mutex> lock(mutex);
        QueueHistory *q = (QueueHistory *)this->history;
        q->iteration([&](const char *line)
                     {
                        oss << line;
                        return true; });
    }
    return oss.str();
}

void Debug::historyIteration(const std::function<bool(const char *)> &callback)
{
    if (this->history)
    {
        std::lock_guard<std::mutex> lock(mutex);
        QueueHistory *q = (QueueHistory *)this->history;
        q->iteration([&](const char *line)
                     { return callback(line); });
    }
}

void Debug::clearLogHistory()
{
    char *line = nullptr;
    if (this->history)
    {
        std::lock_guard<std::mutex> lock(mutex);
        QueueHistory *q = (QueueHistory *)this->history;
        try
        {
            line = q->dequeue();
            delete line;
        }
        catch (const std::exception &e)
        {
        }
    }
}

void Debug::log(Debug::LogType_t type, const char *sourceName, int line, const char *functionName, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::cout << Debug::generate(type, sourceName, line, functionName, format, args);
    va_end(args);
}

void Debug::info(const char *sourceName, int line, const char *functionName, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::cout << Debug::generate(Debug::INFO, sourceName, line, functionName, format, args);
    va_end(args);
}

void Debug::warning(const char *sourceName, int line, const char *functionName, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::cout << Debug::generate(Debug::WARNING, sourceName, line, functionName, format, args);
    va_end(args);
}

void Debug::error(const char *sourceName, int line, const char *functionName, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::cout << Debug::generate(Debug::ERROR, sourceName, line, functionName, format, args);
    va_end(args);
}

void Debug::critical(const char *sourceName, int line, const char *functionName, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::cout << Debug::generate(Debug::CRITICAL, sourceName, line, functionName, format, args);
    va_end(args);
}

std::string Debug::generate(Debug::LogType_t type,
                            const char *sourceName,
                            int line,
                            const char *functionName,
                            const char *format,
                            va_list args)
{
    std::chrono::time_point<std::chrono::system_clock> tnow = std::chrono::system_clock::now();
    std::time_t now = std::chrono::system_clock::to_time_t(tnow);
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tnow.time_since_epoch()) % 1000;
    std::tm localTime;
#if defined(_MSC_VER)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    va_list argsCopy;
    va_copy(argsCopy, args);
    int size = std::vsnprintf(nullptr, 0, format, argsCopy) + 1;
    va_end(argsCopy);

    std::vector<char> buffer(size);
    std::vsnprintf(buffer.data(), size, format, args);

    std::ostringstream oss;
    oss << "[" << std::put_time(&localTime, "%y%m%d_%H%M%S") << '.' << std::setw(3) << std::setfill('0') << ms.count() << "] [" << Debug::logTypeToString(type) << "]: "
        << (sourceName ? std::string(Debug::extractFileName(sourceName)) + (line > 0 ? ":" + std::to_string(line) + " → " : " → ") : "") << Debug::extractFunctionName(functionName) << ": " << buffer.data();

    return oss.str();
}

std::string Debug::generate(LogType_t type,
                            const char *sourceName,
                            int line,
                            const char *functionName,
                            const char *format,
                            ...)
{
    va_list args;
    va_start(args, format);
    std::string result = Debug::generate(type, sourceName, line, functionName, format, args);
    va_end(args);
    return result;
}