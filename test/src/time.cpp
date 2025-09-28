#include "modules.hpp"

void parseTimeCheck(const char *timestr, TimeUtils::TIME_FORMAT format, bool isDateOnly = false, bool isTimeOnly = false, bool isWithGMTOff = false)
{
    struct std::tm result;

    CHECK(TimeUtils::parse(&result, timestr, format) == true);
    if (!isTimeOnly)
    {
        CHECK(result.tm_year == 2025 - 1900);
        CHECK(result.tm_mon == 8);
        CHECK(result.tm_mday == 28);
    }
    if (!isDateOnly)
    {
        CHECK(result.tm_hour == 14);
        CHECK(result.tm_min == 45);
        CHECK(result.tm_sec == 12);
    }
    if (isWithGMTOff)
    {
        CHECK(result.tm_gmtoff == 7);
    }
}

TEST_CASE("Common time getter")
{
    struct std::tm result;
    std::time_t ref = 0;

    TimeUtils::nowUTC(&result);
    ref = std::time(nullptr) - std::mktime(&result);
    ref -= result.tm_gmtoff;
    CHECK((ref == 0 || ref == 1));

    TimeUtils::nowLocal(&result);
    ref = std::time(nullptr) - std::mktime(&result);
    CHECK((ref == 0 || ref == 1));
}

TEST_CASE("Time formater")
{
    struct std::tm result;
    TimeUtils::nowLocal(&result);

    result.tm_year = 2025 - 1900;
    result.tm_mon = 8;
    result.tm_mday = 28;
    result.tm_hour = 14;
    result.tm_min = 45;
    result.tm_sec = 12;

    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_ISO_DATE) == "2025-09-28");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_ISO_DATETIME) == "2025-09-28 14:45:12");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_TIME_24H) == "14:45:12");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_TIME_12H) == "02:45:12 PM");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_LOG_FILE) == "2025-09-28_14-45-12");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_DATE_LONG) == "Sunday, September 28, 2025");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_DATE_SHORT) == "28/09/2025");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_DATE_US) == "09-28-2025");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_DATE_COMPACT) == "20250928");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_RFC_1123) == "Sun, 28 Sep 2025 14:45:12 GMT");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_RFC_3339) == "2025-09-28T14:45:12+0700");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_SQL_DATETIME) == "2025-09-28 14:45:12");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_SQL_DATE) == "2025-09-28");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_TIMEZONE_OFFSET) == "2025-09-28 14:45:12 +0700");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_WEEKDAY_SHORT) == "Sun");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_MONTH_SHORT) == "Sep");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_HOUR_MIN) == "14:45");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_FILENAME_SAFE) == "2025-09-28__14-45-12");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_YEAR_ONLY) == "2025");
    CHECK(TimeUtils::format(&result, TimeUtils::TIME_FORMAT_MONTH_YEAR) == "September 2025");
}

TEST_CASE("Time parser")
{
    parseTimeCheck("2025-09-28", TimeUtils::TIME_FORMAT_ISO_DATETIME, true);
    parseTimeCheck("2025-09-28 14:45:12", TimeUtils::TIME_FORMAT_ISO_DATETIME);
    parseTimeCheck("14:45:12", TimeUtils::TIME_FORMAT_TIME_24H, false, true);
    parseTimeCheck("02:45:12 PM", TimeUtils::TIME_FORMAT_TIME_12H, false, true);
    parseTimeCheck("2025-09-28_14-45-12", TimeUtils::TIME_FORMAT_LOG_FILE);
    parseTimeCheck("Sunday, September 28, 2025", TimeUtils::TIME_FORMAT_DATE_LONG, true);
    parseTimeCheck("28/09/2025", TimeUtils::TIME_FORMAT_DATE_SHORT, true);
    parseTimeCheck("09-28-2025", TimeUtils::TIME_FORMAT_DATE_US, true);
    parseTimeCheck("20250928", TimeUtils::TIME_FORMAT_DATE_COMPACT, true);
    parseTimeCheck("Sun, 28 Sep 2025 14:45:12 GMT", TimeUtils::TIME_FORMAT_RFC_1123);
    /* disabled: not supported
       parseTimeCheck("2025-09-28T14:45:12+0700", TimeUtils::TIME_FORMAT_RFC_3339, false, false, true); */
    parseTimeCheck("2025-09-28 14:45:12", TimeUtils::TIME_FORMAT_SQL_DATETIME);
    parseTimeCheck("2025-09-28", TimeUtils::TIME_FORMAT_SQL_DATE, true);
    /* disabled: not supported
       parseTimeCheck("2025-09-28 14:45:12 +0700", TimeUtils::TIME_FORMAT_TIMEZONE_OFFSET, false, false, true); */
    parseTimeCheck("2025-09-28__14-45-12", TimeUtils::TIME_FORMAT_FILENAME_SAFE);
}