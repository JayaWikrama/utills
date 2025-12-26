/*
 * $Id: debug.hpp, v 1.2.0 2025/09/20 21:28:23 Jaya Wikrama Exp $
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

/**
 * @file
 * @brief Class for customize debug output.
 *
 * This file contains functions that can be used to print custom debug message
 * on terminal and automatically save last 1000 debug line in internal object.
 *
 * @version 1.0.0
 * @date 2024-12-26
 * @author Jaya Wikrama
 */

#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <cstdarg>
#include <deque>

template <typename T>
class Queue;

class Debug
{
private:
    std::vector<std::string> confidential;

    static std::size_t maxLineLogs;
    static std::deque<std::string> history;
    static std::mutex mutex;

public:
    enum LogType_t
    {
        INFO = 0,
        WARNING = 1,
        ERROR = 2,
        CRITICAL = 3
    };

    Debug();
    ~Debug();

    void cache(const std::string &payload);

    void log(LogType_t type, const char *functionName, const char *format, ...);
    void info(const char *functionName, const char *format, ...);
    void warning(const char *functionName, const char *format, ...);
    void error(const char *functionName, const char *format, ...);
    void critical(const char *functionName, const char *format, ...);

    void setConfidential(const std::string &confidential);

    static void setMaxLinesLogCache(std::size_t max);
    static void clearLogHistory();
    static std::string getLogHistory();
    static void historyIteration(const std::function<bool(const char *)> &callback);

    static void log(LogType_t type, const char *sourceName, int line, const char *functionName, const char *format, ...);
    static void info(const char *sourceName, int line, const char *functionName, const char *format, ...);
    static void warning(const char *sourceName, int line, const char *functionName, const char *format, ...);
    static void error(const char *sourceName, int line, const char *functionName, const char *format, ...);
    static void critical(const char *sourceName, int line, const char *functionName, const char *format, ...);

    static std::string generate(LogType_t type,
                                const char *sourceName,
                                int line,
                                const char *functionName,
                                const char *format,
                                va_list args);
    static std::string generate(LogType_t type,
                                const char *sourceName,
                                int line,
                                const char *functionName,
                                const char *format,
                                ...);

protected:
    std::string hideConfidential(const std::string &input) const;

    size_t getMaxLineLogs();
    size_t getHistoriesNumber();
    std::string generate(LogType_t type,
                         const char *functionName,
                         const char *format,
                         va_list args);
    static const char logTypeToChar(LogType_t type);
    static const char *extractFileName(const char *fileName);
};

#endif