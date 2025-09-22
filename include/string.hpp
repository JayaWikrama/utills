#ifndef __STRING_UTILS_HPP__
#define __STRING_UTILS_HPP__

#include <string>
#include <vector>
#include <map>

class StringUtils
{
public:
    static std::string toUpper(const std::string &str);
    static std::string toLower(const std::string &str);

    static std::string trimLeft(const std::string &str);
    static std::string trimRight(const std::string &str);
    static std::string trim(const std::string &str);

    static std::string replaceAll(std::string str, const std::string &from, const std::string &to);
    static std::string toHex(const std::string &input);
    static std::string fromHex(const std::string &hex);

    static unsigned int toIPV4(const std::string &ipv4);
    static std::string fromIPV4(unsigned int ipv4);
    static std::string fromIPV4(const unsigned char *ipv4);

    static std::string urlQueryParamDecode(const char *param, size_t sz);
    static std::string urlQueryParamDecode(const std::string &param);
    static std::string urlQueryParamToJSONString(const char *param, size_t sz);
    static std::string urlQueryParamToJSONString(const std::string &param);

    static std::string fromFile(const std::string &path);

    static std::vector<std::string> split(const std::string &str, char delimiter);
    static std::string join(const std::vector<std::string> &parts, const std::string &delimiter);
    static std::string replacePlaceholder(std::string str, const std::map<std::string, std::string> &values);
    static std::string replacePlaceholder(std::string str, const std::vector<std::string> &values);
};

#endif