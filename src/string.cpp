#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include "string.hpp"

std::string StringUtils::toUpper(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c)
                   { return std::toupper(c); });
    return result;
}

std::string StringUtils::toLower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return result;
}

std::string StringUtils::trimLeft(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string StringUtils::trimRight(const std::string &str)
{
    size_t end = str.find_last_not_of(" \t\n\r");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::string StringUtils::trim(const std::string &str)
{
    return StringUtils::trimRight(StringUtils::trimLeft(str));
}

std::string StringUtils::replaceAll(std::string str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return str;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string StringUtils::toHex(const std::string &input)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : input)
    {
        oss << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

std::string StringUtils::fromHex(const std::string &hex)
{
    if (hex.size() % 2 != 0)
        throw std::invalid_argument("Invalid hex length");
    std::string output;
    output.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2)
    {
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(strtol(byteString.c_str(), nullptr, 16));
        output.push_back(byte);
    }
    return output;
}

unsigned int StringUtils::toIPV4(const std::string &ipv4)
{
    std::istringstream ss(ipv4);
    std::string token;
    unsigned int result = 0;
    unsigned char ip[4];
    int i = 0;

    while (std::getline(ss, token, '.'))
    {
        if (i >= 4)
            return false;
        try
        {
            int part = std::stoi(token);
            if (part < 0 || part > 255)
                return false;
            ip[i++] = static_cast<unsigned char>(part);
        }
        catch (...)
        {
            return 0;
        }
    }
    if (i != 4)
        return 0;
    memcpy(&result, ip, 4);
    return result;
}

std::string StringUtils::fromIPV4(unsigned int ipv4)
{
    return StringUtils::fromIPV4((const unsigned char *)&ipv4);
}

std::string StringUtils::fromIPV4(const unsigned char *ipv4)
{
    char buffer[16];
    memset(buffer, 0x00, sizeof(buffer));
    snprintf(buffer, 16, "%u.%u.%u.%u", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
    return std::string(buffer);
}

std::string StringUtils::urlQueryParamDecode(const char *param, size_t sz)
{
    std::string result;
    for (size_t i = 0; i < sz; i++)
    {
        if (param[i] == '%' && i + 2 < sz)
        {
            char hex[3] = {param[i + 1], param[i + 2], 0};
            result += static_cast<char>(strtol(hex, nullptr, 16));
            i += 2;
        }
        else if (param[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += param[i];
        }
    }
    return result;
}

std::string StringUtils::urlQueryParamDecode(const std::string &param)
{
    return StringUtils::urlQueryParamDecode(param.c_str(), param.length());
}

std::string StringUtils::urlQueryParamToJSONString(const char *param, size_t sz)
{
    std::stringstream json;
    json << "{";

    const char *start = param;
    const char *end = param + sz;
    bool first = true;

    while (start < end)
    {
        const char *eq = (const char *)memchr(start, '=', end - start);
        const char *amp = (const char *)memchr(start, '&', end - start);
        if (!eq)
            break;

        size_t keyLen = eq - start;
        size_t valLen = 0;

        const char *valStart = eq + 1;

        if (amp)
        {
            valLen = amp - valStart;
        }
        else
        {
            valLen = end - valStart;
        }

        std::string key = StringUtils::urlQueryParamDecode(start, keyLen);
        std::string val = StringUtils::urlQueryParamDecode(valStart, valLen);

        if (!first)
            json << ",";
        json << "\"" << key << "\":\"" << val << "\"";
        first = false;

        start = amp ? amp + 1 : end;
    }

    json << "}";
    return json.str();
}

std::string StringUtils::urlQueryParamToJSONString(const std::string &param)
{
    return StringUtils::urlQueryParamToJSONString(param.c_str(), param.length());
}

std::string StringUtils::fromFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open PEM file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> StringUtils::split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::string StringUtils::join(const std::vector<std::string> &parts, const std::string &delimiter)
{
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i)
    {
        oss << parts[i];
        if (i != parts.size() - 1)
            oss << delimiter;
    }
    return oss.str();
}

std::string StringUtils::replacePlaceholder(std::string str, const std::map<std::string, std::string> &values)
{
    for (const auto &pair : values)
    {
        const std::string &key = pair.first;
        const std::string &val = pair.second;
        std::string placeholder = "{" + key + "}";
        str = StringUtils::replaceAll(str, placeholder, val);
    }
    return str;
}

std::string StringUtils::replacePlaceholder(std::string str, const std::vector<std::string> &values)
{
    for (size_t i = 0; i < values.size(); ++i)
    {
        std::string placeholder = "{" + std::to_string(i) + "}";
        str = StringUtils::replaceAll(str, placeholder, values[i]);
    }
    return str;
}