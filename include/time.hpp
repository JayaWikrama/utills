#ifndef __TIME_UTILS_HPP__
#define __TIME_UTILS_HPP__

#include <string>
#include <ctime>

class TimeUtils
{
public:
    typedef enum _TIME_FORMAT
    {
        TIME_FORMAT_ISO_DATE = 0,    // 2025-09-28
        TIME_FORMAT_ISO_DATETIME,    // 2025-09-28 14:45:12
        TIME_FORMAT_TIME_24H,        // 14:45:12
        TIME_FORMAT_TIME_12H,        // 02:45:12 PM
        TIME_FORMAT_LOG_FILE,        // 2025-09-28_14-45-12
        TIME_FORMAT_DATE_LONG,       // Sunday, September 28, 2025
        TIME_FORMAT_DATE_SHORT,      // 28/09/2025
        TIME_FORMAT_DATE_US,         // 09-28-2025
        TIME_FORMAT_DATE_COMPACT,    // 20250928
        TIME_FORMAT_RFC_1123,        // Sun, 28 Sep 2025 14:45:12 GMT
        TIME_FORMAT_RFC_3339,        // 2025-09-28T14:45:12+00:00
        TIME_FORMAT_SQL_DATETIME,    // 2025-09-28 14:45:12 (untuk database)
        TIME_FORMAT_SQL_DATE,        // 2025-09-28
        TIME_FORMAT_TIMEZONE_OFFSET, // 2025-09-28 14:45:12 +0700
        TIME_FORMAT_WEEKDAY_SHORT,   // Sun
        TIME_FORMAT_MONTH_SHORT,     // Sep
        TIME_FORMAT_HOUR_MIN,        // 14:45
        TIME_FORMAT_FILENAME_SAFE,   // 2025-09-28__14-45-12 (extra safe for file)
        TIME_FORMAT_YEAR_ONLY,       // 2025
        TIME_FORMAT_MONTH_YEAR       // September 2025
    } TIME_FORMAT;

    static void nowLocal(std::tm *result);
    static void nowUTC(std::tm *result);

    static std::string format(const std::tm *tmTime, const char *formatStr);
    static std::string format(const std::tm *tmTime, TIME_FORMAT format);
    static bool parse(std::tm *result, const char *dateStr, const char *formatStr);
    static bool parse(std::tm *result, const char *dateStr, TIME_FORMAT format);

    static void addSeconds(std::tm *time, int seconds);
    static void addDays(std::tm *time, int days);

    static int diffSeconds(const std::tm *a, const std::tm *b);
    static int diffDays(const std::tm *a, const std::tm *b);

    static std::time_t toEpoch(const std::tm *time);
    static void fromEpoch(std::tm *result, std::time_t epoch);
    static bool isValidDate(int year, int month, int day);

    static std::time_t getDateEpoch(std::time_t time);

private:
    static const char *getFormat(TIME_FORMAT format);
};

#endif