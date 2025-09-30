#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include "time.hpp"

void TimeUtils::nowLocal(std::tm *result)
{
    std::time_t t = std::time(nullptr);
    memcpy(result, std::localtime(&t), sizeof(std::tm));
}

void TimeUtils::nowUTC(std::tm *result)
{
    std::time_t t = std::time(nullptr);
    memcpy(result, std::gmtime(&t), sizeof(std::tm));
}

std::string TimeUtils::format(const std::tm *tmTime, const char *formatStr)
{
    std::ostringstream oss;
    oss << std::put_time(tmTime, formatStr);
    return oss.str();
}

std::string TimeUtils::format(const std::tm *tmTime, TimeUtils::TIME_FORMAT format)
{
    return TimeUtils::format(tmTime, TimeUtils::getFormat(format));
}

bool TimeUtils::parse(std::tm *result, const char *dateStr, const char *formatStr)
{
    memset(result, 0x00, sizeof(std::tm));
    std::istringstream ss(dateStr);
    ss >> std::get_time(result, formatStr);
    if (ss.fail())
    {
        return false;
    }
    return true;
}

bool TimeUtils::parse(std::tm *result, const char *dateStr, TimeUtils::TIME_FORMAT format)
{
    return TimeUtils::parse(result, dateStr, TimeUtils::getFormat(format));
}

void TimeUtils::addSeconds(std::tm *time, int seconds)
{
    std::time_t tt = std::mktime(time);
    tt += seconds;
    memcpy(time, std::localtime(&tt), sizeof(std::tm));
}

void TimeUtils::addDays(std::tm *time, int days)
{
    TimeUtils::addSeconds(time, days * 86400);
}

int TimeUtils::diffSeconds(const std::tm *a, const std::tm *b)
{
    std::time_t ta = std::mktime(const_cast<std::tm *>(a));
    std::time_t tb = std::mktime(const_cast<std::tm *>(b));
    return static_cast<int>(std::difftime(ta, tb));
}

int TimeUtils::diffDays(const std::tm *a, const std::tm *b)
{
    return TimeUtils::diffSeconds(a, b) / 86400.0;
}

std::time_t TimeUtils::toEpoch(const std::tm *time)
{
    return std::mktime(const_cast<std::tm *>(time));
}

void TimeUtils::fromEpoch(std::tm *result, std::time_t epoch)
{
    memcpy(result, std::localtime(&epoch), sizeof(std::tm));
}

bool TimeUtils::isValidDate(int year, int month, int day)
{
    if (month < 1 || month > 12 || day < 1)
        return false;
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[month - 1];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
    {
        maxDay = 29;
    }
    return day <= maxDay;
}

std::time_t TimeUtils::getDateEpoch(std::time_t time)
{
    struct std::tm tmr;
    memcpy(&tmr, std::localtime(&time), sizeof(tmr));
    tmr.tm_hour = 0;
    tmr.tm_min = 0;
    tmr.tm_sec = 0;
    return std::mktime(&tmr);
}

const char *TimeUtils::getFormat(TimeUtils::TIME_FORMAT format)
{
    static const char *formatCollection[] = {
        "%Y-%m-%d",
        "%Y-%m-%d %H:%M:%S",
        "%H:%M:%S",
        "%I:%M:%S %p",
        "%Y-%m-%d_%H-%M-%S",
        "%A, %B %d, %Y",
        "%d/%m/%Y",
        "%m-%d-%Y",
        "%Y%m%d",
        "%a, %d %b %Y %H:%M:%S GMT",
        "%Y-%m-%dT%H:%M:%S%z",
        "%Y-%m-%d %H:%M:%S",
        "%Y-%m-%d",
        "%Y-%m-%d %H:%M:%S %z",
        "%a",
        "%b",
        "%H:%M",
        "%Y-%m-%d__%H-%M-%S",
        "%Y",
        "%B %Y"};
    return formatCollection[static_cast<int>(format)];
}