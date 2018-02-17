#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cassert>

namespace smart_enum
{
    inline std::string trimWhitespace(std::string str)
    {
        // trim trailing whitespace
        size_t endPos = str.find_last_not_of(" \t");
        if(std::string::npos != endPos)
        {
            str = str.substr(0, endPos + 1);
        }

        // trim leading spaces
        size_t startPos = str.find_first_not_of(" \t");
        if(std::string::npos != startPos)
        {
            str = str.substr(startPos);
        }

        return str;
    }

    inline std::string extractEntry(std::string& valuesString)
    {
        std::string result;
        size_t nextCommaPos = valuesString.find(',');

        if(nextCommaPos != std::string::npos)
        {
            std::string segment = valuesString.substr(0, nextCommaPos);
            valuesString.erase(0, nextCommaPos + 1);
            result = trimWhitespace(segment);
        }
        else
        {
            result = trimWhitespace(valuesString);
            valuesString = "";
        };
        return result;
    };

    inline std::unordered_map<int32_t, std::string> makeEnumNameMap(std::string enumValuesString)
    {
        std::unordered_map<int32_t, std::string> nameMap;

        int32_t currentEnumValue = 0;
        while(enumValuesString != "")
        {
            std::string currentEnumEntry = extractEntry(enumValuesString);

            size_t equalSignPos = currentEnumEntry.find('=');
            if(equalSignPos != std::string::npos)
            {
                std::string rightHandSide = currentEnumEntry.substr(equalSignPos + 1);
                currentEnumValue = std::stoi(rightHandSide);
                currentEnumEntry.erase(equalSignPos);
            }

            currentEnumEntry = trimWhitespace(currentEnumEntry);

            nameMap[currentEnumValue] = currentEnumEntry;

            currentEnumValue++;
        }

        return nameMap;
    }

    template<typename Type>
    std::vector<Type> makeEnumList(std::string enumValuesString)
    {
        std::vector<Type> enumList;

        int32_t currentEnumValue = 0;
        while(enumValuesString != "")
        {
            std::string currentEnumEntry = extractEntry(enumValuesString);

            size_t equalSignPos = currentEnumEntry.find('=');
            if(equalSignPos != std::string::npos)
            {
                std::string rightHandSide = currentEnumEntry.substr(equalSignPos + 1);
                currentEnumValue = std::stoi(rightHandSide);
                currentEnumEntry.erase(equalSignPos);
            }

            currentEnumEntry = trimWhitespace(currentEnumEntry);

            enumList.push_back(static_cast<Type>(currentEnumValue));

            currentEnumValue++;
        }

        return enumList;
    }

    inline std::unordered_map<std::string, int32_t> makeEnumValuesMap(std::string enumValuesString)
    {
        std::unordered_map<std::string, int32_t> nameMap;

        int32_t currentEnumValue = 0;
        while(enumValuesString != "")
        {
            std::string currentEnumEntry = extractEntry(enumValuesString);

            size_t equalSignPos = currentEnumEntry.find('=');
            if(equalSignPos != std::string::npos)
            {
                std::string rightHandSide = currentEnumEntry.substr(equalSignPos + 1);
                currentEnumValue = std::stoi(rightHandSide);
                currentEnumEntry.erase(equalSignPos);
            }

            currentEnumEntry = trimWhitespace(currentEnumEntry);

            nameMap[currentEnumEntry] = currentEnumValue;

            currentEnumValue++;
        }

        return nameMap;
    }

    inline std::string toLower(const std::string& str) {
      std::string s;

      for(const auto& c : str) {
        s.push_back(tolower(c));
      }

      return s;
    }
    inline bool string_case_compare(const std::string& lhs, const std::string& rhs) {
      return toLower(lhs) == toLower(rhs);
    }
}

#define smart_enum(Type, ...) enum Type { __VA_ARGS__}; \
    static std::unordered_map<int32_t, std::string> Type##_enum_names = smart_enum::makeEnumNameMap(#__VA_ARGS__);\
    static std::vector<Type> Type##_list = smart_enum::makeEnumList<Type>(#__VA_ARGS__);\
    static std::unordered_map<std::string, int32_t> Type##_enum_values = smart_enum::makeEnumValuesMap(#__VA_ARGS__);\
    \
    inline const std::string& Type##_to_string(Type value) \
    { \
        return Type##_enum_names.at((int32_t)value);\
    } \
    \
    inline const Type Type##_to_enum(const std::string& name)\
    {\
        return static_cast<Type>(Type##_enum_values.at(name));\
    }\

#define smart_enum_class(Type, ...) enum class Type { __VA_ARGS__}; \
    static std::unordered_map<int32_t, std::string> Type##_enum_names = smart_enum::makeEnumNameMap(#__VA_ARGS__);\
    static std::vector<Type> Type##_list = smart_enum::makeEnumList<Type>(#__VA_ARGS__);\
    static std::unordered_map<std::string, int32_t> Type##_enum_values = smart_enum::makeEnumValuesMap(#__VA_ARGS__);\
    \
    inline const std::string& to_string(Type value) \
    { \
        return Type##_enum_names.at((int32_t)value);\
    } \
    \
    inline std::ostream& operator<<(std::ostream& outStream, Type value)\
    {\
        outStream << to_string(value);\
        return outStream;\
    }\
    \
    inline const Type Type##_to_enum_class(const std::string& name)\
    {\
        auto it = std::find_if(Type##_enum_values.begin(), Type##_enum_values.end(), [&name](const std::pair<std::string, int32_t>& pair) { \
          return smart_enum::string_case_compare(name, pair.first); \
        }); \
        if(it != Type##_enum_values.end()) { \
            \
        } else { \
          return static_cast<Type>(Type##_enum_values.begin()->second); \
        } \
        return static_cast<Type>(Type##_enum_values.at(name));\
    }
