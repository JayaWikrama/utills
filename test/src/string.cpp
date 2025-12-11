#include "modules.hpp"

TEST_CASE("To Upper")
{
    std::string s = StringUtils::toUpper("12werty78uuII44as11");
    CHECK(s.compare("12WERTY78UUII44AS11") == 0);
}

TEST_CASE("To Lower")
{
    std::string s = StringUtils::toLower("12WERTy78uUII44AS11");
    CHECK(s.compare("12werty78uuii44as11") == 0);
}

TEST_CASE("Trim Left")
{
    std::string s = StringUtils::trimLeft("      12WERTy78uUII44AS11");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimLeft("\t      12WERTy78uUII44AS11");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimLeft("\n      12WERTy78uUII44AS11");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimLeft("\r      12WERTy78uUII44AS11");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimLeft("\r\n\t      12WERTy78uUII44AS11");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
}

TEST_CASE("Trim Right")
{
    std::string s = StringUtils::trimRight("12WERTy78uUII44AS11      ");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimRight("12WERTy78uUII44AS11\t      ");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimRight("12WERTy78uUII44AS11\n      ");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimRight("12WERTy78uUII44AS11\r      ");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimRight("12WERTy78uUII44AS11\r\n\t      ");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
    s = StringUtils::trimRight("12WERTy78uUII44AS11\r\n\t      \r\n");
    CHECK(s.compare("12WERTy78uUII44AS11") == 0);
}

TEST_CASE("Replacement")
{
    std::string b = "ini token itu token semua token";
    std::string s = StringUtils::replaceAll(b, "token", "tiken");
    CHECK(s.compare("ini tiken itu tiken semua tiken") == 0);
    CHECK(b.compare("ini tiken itu tiken semua tiken") == 0);
    s = StringUtils::replaceAll(b, "token", "tiken");
    CHECK(s.compare("ini tiken itu tiken semua tiken") == 0);
    CHECK(b.compare("ini tiken itu tiken semua tiken") == 0);
}

TEST_CASE("Replacement")
{
    std::string b = "ini token itu token semua token";
    std::string s = StringUtils::replaceAll(b, "token", "tiken");
    CHECK(s.compare("ini tiken itu tiken semua tiken") == 0);
    CHECK(b.compare("ini tiken itu tiken semua tiken") == 0);
    s = StringUtils::replaceAll(b, "token", "tiken");
    CHECK(s.compare("ini tiken itu tiken semua tiken") == 0);
    CHECK(b.compare("ini tiken itu tiken semua tiken") == 0);
}

TEST_CASE("To Hex")
{
    std::string s = StringUtils::toHexString("123");
    CHECK(s.compare("313233") == 0);
    std::vector<unsigned char> hex = StringUtils::toHexBin("123");
    CHECK(hex.empty() == true);
    hex = StringUtils::toHexBin(s);
    CHECK(hex.size() == 3);
    CHECK(std::equal(hex.begin(), hex.end(), std::begin(std::initializer_list<unsigned char>{0x31, 0x32, 0x33})) == true);
    hex = StringUtils::toHexBin("003132333435363738390a0b0c0d0E0fFF");
    CHECK(hex.size() == 17);
    CHECK(std::equal(hex.begin(), hex.end(), std::begin(std::initializer_list<unsigned char>{0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF})) == true);
}

TEST_CASE("From Hex")
{
    char tmp[64];
    StringUtils::fromHex(tmp, 0xE2);
    CHECK(memcmp(tmp, "E2", 2) == 0);
    StringUtils::fromHex(tmp, {0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF});
    CHECK(memcmp(tmp, "003132333435363738390A0B0C0D0E0FFF", 34) == 0);
    std::string s = StringUtils::fromHex(0xE2);
    CHECK(s.compare("E2") == 0);
    s = StringUtils::fromHex({0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF});
    CHECK(s.compare("003132333435363738390A0B0C0D0E0FFF") == 0);
    s = StringUtils::fromHexString("30303331333233333334333533363337333833393041304230433044304530464646");
    CHECK(s.compare("003132333435363738390A0B0C0D0E0FFF") == 0);
    s = StringUtils::fromHexToPrettyString({0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF});
    CHECK(s.compare("00 31 32 33 34 35 36 37 38 39 0A 0B 0C 0D 0E 0F FF") == 0);
    const char *cstr = StringUtils::fromHexToPrettyString<1>((unsigned char *)"\x02\x03\xF1\xA5", 4);
    CHECK(strcmp(cstr, "02") == 0);
    cstr = StringUtils::fromHexToPrettyString<2>((unsigned char *)"\x02\x03\xF1\xA5", 4);
    CHECK(strcmp(cstr, "02 03") == 0);
    cstr = StringUtils::fromHexToPrettyString<3>((unsigned char *)"\x02\x03\xF1\xA5", 4);
    CHECK(strcmp(cstr, "02 03 F1") == 0);
    cstr = StringUtils::fromHexToPrettyString<4>((unsigned char *)"\x02\x03\xF1\xA5", 4);
    CHECK(strcmp(cstr, "02 03 F1 A5") == 0);
    cstr = StringUtils::fromHexToPrettyString<5>((unsigned char *)"\x02\x03\xF1\xA5", 4);
    CHECK(strcmp(cstr, "02 03 F1 A5") == 0);
}

TEST_CASE("IPV4")
{
    int ipv4 = StringUtils::toIPV4("0.0.0.0");
    CHECK(ipv4 == 0);
    ipv4 = StringUtils::toIPV4("255.255.255.255");
    CHECK(ipv4 == 0xFFFFFFFF);
    ipv4 = StringUtils::toIPV4("127.0.0.1");
    CHECK(ipv4 == 0x0100007F);
    std::string s = StringUtils::fromIPV4(ipv4);
    CHECK(s.compare("127.0.0.1") == 0);
    s = StringUtils::fromIPV4(0xFFFFFFFF);
    CHECK(s.compare("255.255.255.255") == 0);
    s = StringUtils::fromIPV4((const unsigned char *)"\x7F\x00\x00\x01");
    CHECK(s.compare("127.0.0.1") == 0);
}