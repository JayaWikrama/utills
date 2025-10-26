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

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <cstdarg>

#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

typedef void *QueuePtr;

class Debug
{
private:
    size_t maxLineLogs;
    QueuePtr history;
    mutable std::mutex mutex;

public:
    enum LogType_t
    {
        INFO = 0,
        WARNING = 1,
        ERROR = 2,
        CRITICAL = 3
    };

    Debug(size_t maxLineLogs = 0);
    ~Debug();

    void log(LogType_t type, const char *functionName, const char *format, ...);
    std::string getLogHistory() const;
    void historyIteration(const std::function<bool(const char *)> &callback);
    void clearLogHistory();

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
    size_t getMaxLineLogs();
    size_t getHistoriesNumber();
    std::string generate(LogType_t type,
                         const char *functionName,
                         const char *format,
                         va_list args);
#ifdef CXXSTD_17
    static std::string_view logTypeToString(LogType_t type);
    static std::string_view extractFileName(const char *fileName);
    static std::string_view extractFunctionName(const char *functionName);
#else
    static std::string logTypeToString(LogType_t type);
    static std::string extractFileName(const char *fileName);
    static std::string extractFunctionName(const char *functionName);
#endif
};

#endif