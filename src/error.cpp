/**
 * $Id: error.hpp, v1.0.0 2025/10/05 14:43:00 Jaya Wikrama Exp $
 *
 * Copyright (c) 2025 Jaya Wikrama
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "error.hpp"
#include "debug.hpp"

/**
 * @brief Generate error message when a required field is not found.
 *
 * @param src Source file name where the error occurred.
 * @param line Line number in the source file.
 * @param func Function name where the error occurred.
 * @param field Name of the missing field.
 * @param parent Name of the parent object or structure.
 * @return Formatted error message string.
 */
std::string Error::fieldNotFound(const std::string &src, int line, const std::string &func, const std::string &field, const std::string &parent)
{
    if (parent.empty() == false)
        return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "field \"%s\" in \"%s\" is not found", field.c_str(), parent.c_str());
    return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "field \"%s\" is not found", field.c_str());
}

/**
 * @brief Generate error message when a field type is invalid.
 *
 * @param src Source file name where the error occurred.
 * @param line Line number in the source file.
 * @param func Function name where the error occurred.
 * @param expType Expected type of the field.
 * @param field Name of the field being checked.
 * @param parent Name of the parent object or structure.
 * @return Formatted error message string.
 */
std::string Error::fieldTypeInvalid(const std::string &src, int line, const std::string &func, const std::string &expType, const std::string &field, const std::string &parent)
{
    if (parent.empty() == false)
        return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "field \"%s\" type in \"%s\" is not %s", field.c_str(), parent.c_str(), expType.c_str());
    return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "field \"%s\" type is not %s", field.c_str(), expType.c_str());
}

/**
 * @brief Generate error message for parsing failures.
 *
 * @param src Source file name where the error occurred.
 * @param line Line number in the source file.
 * @param func Function name where the error occurred.
 * @param obj Name of the object being parsed.
 * @param what Description of the parsing error.
 * @return Formatted error message string.
 */
std::string Error::parseError(const std::string &src, int line, const std::string &func, const std::string &obj, const std::string &what)
{
    return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "failed to parse %s: %s", obj.c_str(), what.c_str());
}

/**
 * @brief Generate error message when an object is out of range.
 *
 * @param src Source file name where the error occurred.
 * @param line Line number in the source file.
 * @param func Function name where the error occurred.
 * @param obj Name of the object being checked.
 * @param max Maximum allowed value (-1 indicates no limit).
 * @return Formatted error message string.
 */
std::string Error::outOfRange(const std::string &src, int line, const std::string &func, const std::string &obj, int max)
{
    if (max < 0)
        return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "%s out of range", obj.c_str());
    return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "%s out of range (max: %d)", obj.c_str(), max);
}

/**
 * @brief Generate error message when an object is out of its allowed period.
 *
 * @param src Source file name where the error occurred.
 * @param line Line number in the source file.
 * @param func Function name where the error occurred.
 * @param obj Name of the object being checked.
 * @return Formatted error message string.
 */
std::string Error::outOfPeriod(const std::string &src, int line, const std::string &func, const std::string &obj)
{
    return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "%s out of period", obj.c_str());
}

/**
 * @brief Generate error message when an object has not been set yet.
 *
 * @param src Source file name where the error occurred.
 * @param line Line number in the source file.
 * @param func Function name where the error occurred.
 * @param obj Name of the object that is not set.
 * @return Formatted error message string.
 */
std::string Error::notYetSet(const std::string &src, int line, const std::string &func, const std::string &obj)
{
    return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "%s is not set yet", obj.c_str());
}

/**
 * @brief Generate a generic/common error message.
 *
 * @param src Source file name where the error occurred.
 * @param line Line number in the source file.
 * @param func Function name where the error occurred.
 * @param msg The error message content.
 * @return Formatted error message string.
 */
std::string Error::common(const std::string &src, int line, const std::string &func, const std::string &msg)
{
    return Debug::generate(Debug::ERROR, src.c_str(), line, func.c_str(), "%s", msg.c_str());
}