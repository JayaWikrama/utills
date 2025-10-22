/**
 * $Id: error.hpp, v1.0.1 2025/10/19 11:12:00 Jaya Wikrama Exp $
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
 *
 * This module provides a collection of static methods for creating
 * formatted error messages, typically used for logging and debugging
 * within the system. Each method returns a string containing the
 * generated error message.
 *
 * @note
 * Release Notes:
 * - v1.0.1 (2025-10-19):
 *   Handle empty parrent.
 *
 * - v1.0.0 (2025-10-05):
 *   first release.
 */

#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include <string>

/**
 * @class Error
 * @brief Utility class for generating error messages.
 *
 * The Error class provides several static methods to generate
 * descriptive error messages for common error cases such as missing fields,
 * invalid types, parsing failures, out-of-range conditions, and more.
 */
class Error
{
public:
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
    static std::string fieldNotFound(const std::string &src, int line, const std::string &func, const std::string &field, const std::string &parent = "");

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
    static std::string fieldTypeInvalid(const std::string &src, int line, const std::string &func, const std::string &expType, const std::string &field, const std::string &parent = "");

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
    static std::string parseError(const std::string &src, int line, const std::string &func, const std::string &obj, const std::string &what);

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
    static std::string outOfRange(const std::string &src, int line, const std::string &func, const std::string &obj, int max = -1);

    /**
     * @brief Generate error message when an object is out of its allowed period.
     *
     * @param src Source file name where the error occurred.
     * @param line Line number in the source file.
     * @param func Function name where the error occurred.
     * @param obj Name of the object being checked.
     * @return Formatted error message string.
     */
    static std::string outOfPeriod(const std::string &src, int line, const std::string &func, const std::string &obj);

    /**
     * @brief Generate error message when an object has not been set yet.
     *
     * @param src Source file name where the error occurred.
     * @param line Line number in the source file.
     * @param func Function name where the error occurred.
     * @param obj Name of the object that is not set.
     * @return Formatted error message string.
     */
    static std::string notYetSet(const std::string &src, int line, const std::string &func, const std::string &obj);

    /**
     * @brief Generate a generic/common error message.
     *
     * @param src Source file name where the error occurred.
     * @param line Line number in the source file.
     * @param func Function name where the error occurred.
     * @param msg The error message content.
     * @return Formatted error message string.
     */
    static std::string common(const std::string &src, int line, const std::string &func, const std::string &msg);
};

#endif