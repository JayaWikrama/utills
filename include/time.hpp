#ifndef __TIME_UTILS_HPP__
#define __TIME_UTILS_HPP__

#include <string>
#include <ctime>

class TimeUtils
{
public:
    static void nowLocal(std::tm *result);
    static void nowUTC(std::tm *result);

    static std::string format(const std::tm *tmTime, const char *formatStr);
    static bool parse(std::tm *result, const char *dateStr, const char *formatStr);

    static void addSeconds(std::tm *time, int seconds);
    static void addDays(std::tm *time, int days);

    static int diffSeconds(const std::tm *a, const std::tm *b);
    static int diffDays(const std::tm *a, const std::tm *b);

    static std::time_t toEpoch(const std::tm *time);
    static void fromEpoch(std::tm *result, std::time_t epoch);
    static bool isValidDate(int year, int month, int day);

    static std::time_t getDateEpoch(std::time_t time);
};

#endif