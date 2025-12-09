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
#include <array>
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

std::string Debug::hideConfidential(const std::string &input) const
{
    std::string result = input;
    for (const std::string &conf : this->confidential)
    {
        size_t pos = input.find(conf);
        if (pos != std::string::npos)
        {
            result.replace(pos, conf.length(), "*****");
        }
    }
    return result;
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

const char Debug::logTypeToChar(Debug::LogType_t type)
{
    static const char *logTypeChar = "IWEC";
    return logTypeChar[static_cast<int>(type)];
}

const char *Debug::extractFileName(const char *fileName)
{
    const char *slash = strrchr(fileName, '/');
    if (!slash)
        slash = strrchr(fileName, '\\');
    return slash ? slash + 1 : fileName;
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
    std::string logPayload = this->generate(type, functionName, format, args);
    va_end(args);

    std::string logEntry = this->hideConfidential(logPayload);

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

void Debug::setConfidential(const std::string &confidential)
{
    this->confidential.push_back(confidential);
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
    std::tm localTime{};
#if defined(_MSC_VER)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::array<char, 1024> localBuf{};
    std::size_t offset = 27 + strlen(functionName);

    if (sourceName)
    {
        char sourcen[128]{};
        snprintf(sourcen, 127, "%s:%d → ", Debug::extractFileName(sourceName), line);
        offset += strlen(sourcen);

        std::snprintf(localBuf.data(), offset + 1,
                      "[%02d%02d%02d_%02d%02d%02d.%03ld] [%c]: %s%s: ",
                      (localTime.tm_year % 100),
                      (localTime.tm_mon + 1),
                      localTime.tm_mday,
                      localTime.tm_hour,
                      localTime.tm_min,
                      localTime.tm_sec,
                      static_cast<long>(ms.count()),
                      Debug::logTypeToChar(type), sourcen, functionName);
    }
    else
    {
        std::snprintf(localBuf.data(), offset + 1,
                      "[%02d%02d%02d_%02d%02d%02d.%03ld] [%c]: %s: ",
                      (localTime.tm_year % 100),
                      (localTime.tm_mon + 1),
                      localTime.tm_mday,
                      localTime.tm_hour,
                      localTime.tm_min,
                      localTime.tm_sec,
                      static_cast<long>(ms.count()),
                      Debug::logTypeToChar(type), functionName);
    }

    va_list argsCopy;
    va_copy(argsCopy, args);
    int needed = std::vsnprintf(localBuf.data() + offset, localBuf.size() - offset, format, argsCopy);
    va_end(argsCopy);

    std::string result;

    if (needed < 0)
    {
        std::copy_n("[format-error]\n", 15, localBuf.data() + offset);
        result.assign(localBuf.data(), strlen(localBuf.data()));
    }
    else if (static_cast<size_t>(needed + offset) < localBuf.size())
    {
        result.assign(localBuf.data(), static_cast<std::size_t>(needed + offset));
    }
    else
    {
        result.resize(static_cast<size_t>(needed) + offset + 1);
        std::copy_n(localBuf.begin(), offset, result.begin());
        std::vsnprintf(result.data() + offset, result.size() - offset, format, args);
    }
    return result;
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