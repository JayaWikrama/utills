#include "modules.hpp"

class DebugZeroLinesHistoryTestHelper : public Debug
{
public:
    using Debug::getHistoriesNumber;
    using Debug::getMaxLineLogs;

    std::string gen(LogType_t type, const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        std::string out = this->generate(type, __func__, format, args);
        va_end(args);
        return out;
    }
};

class DebugWithSomeLinesHistoryTestHelper : public Debug
{
public:
    DebugWithSomeLinesHistoryTestHelper() : Debug(3) {}
    using Debug::getHistoriesNumber;
    using Debug::getMaxLineLogs;

    std::string gen(LogType_t type, const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        std::string out = this->generate(type, __func__, format, args);
        va_end(args);
        return out;
    }
};

TEST_CASE("Debug with zero history line size")
{
    DebugZeroLinesHistoryTestHelper debug;

    SUBCASE("Check n-lines")
    {
        debug.log(Debug::INFO, __func__, "start\n");
        CHECK(debug.getMaxLineLogs() == 0);
        CHECK(debug.getHistoriesNumber() == 0);
    }

    SUBCASE("Check output log")
    {
        std::string v = debug.gen(Debug::INFO, "test %d %.02f %05llu\n", 10, 0.321, 221);
        CHECK(v.length() == 49);
        CHECK(v.at(0) == '[');
        for (int i = 1; i < 7; i++)
        {
            CHECK(std::isdigit(v.at(i)));
        }
        CHECK(v.at(7) == '_');
        for (int i = 8; i < 14; i++)
        {
            CHECK(std::isdigit(v.at(i)));
        }
        CHECK(v.at(14) == '.');
        for (int i = 15; i < 18; i++)
        {
            CHECK(std::isdigit(v.at(i)));
        }
        CHECK(v.at(18) == ']');
        CHECK(v.compare(20, 29, "[I]: gen: test 10 0.32 00221\n", 0, 29) == 0);
    }
}

TEST_CASE("Debug with 3 history lines size")
{
    DebugWithSomeLinesHistoryTestHelper debug;

    SUBCASE("Check n-lines")
    {
        debug.log(Debug::INFO, __func__, "start\n");
        CHECK(debug.getMaxLineLogs() == 3);
        CHECK(debug.getHistoriesNumber() == 1);
    }

    SUBCASE("Check output log")
    {
        std::string v = debug.gen(Debug::INFO, "test %d %.02f %05llu\n", 10, 0.321, 221);
        CHECK(v.length() == 49);
        CHECK(v.at(0) == '[');
        for (int i = 1; i < 7; i++)
        {
            CHECK(std::isdigit(v.at(i)));
        }
        CHECK(v.at(7) == '_');
        for (int i = 8; i < 14; i++)
        {
            CHECK(std::isdigit(v.at(i)));
        }
        CHECK(v.at(14) == '.');
        for (int i = 15; i < 18; i++)
        {
            CHECK(std::isdigit(v.at(i)));
        }
        CHECK(v.at(18) == ']');
        CHECK(v.compare(20, 29, "[I]: gen: test 10 0.32 00221\n", 0, 29) == 0);
    }

    SUBCASE("Check history")
    {
        for (int i = 0; i < 20; i++)
        {
            debug.log(Debug::INFO, "hcheck", "%d\n", i);
        }
        std::string history = debug.getLogHistory();
        CHECK(debug.getHistoriesNumber() == 3);
        CHECK(history.compare(20, 16, "[I]: hcheck: 17\n", 0, 16) == 0);

        int idx = 0;
        debug.historyIteration([&](const char *line)
                               {
                                    switch (idx) {
                                        case 0:
                                            CHECK(memcmp(line + 20, "[I]: hcheck: 17\n", 16) == 0);
                                            break;
                                        case 1:
                                            CHECK(memcmp(line + 20, "[I]: hcheck: 18\n", 16) == 0);
                                            break;
                                        case 2:
                                            CHECK(memcmp(line + 20, "[I]: hcheck: 19\n", 16) == 0);
                                            break;
                                    }
                                    idx++;
                                    return true; });
        CHECK(idx == 3);
    }
}
