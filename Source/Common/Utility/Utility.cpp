/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Common/Utility/Utility.hpp"

std::string Common::GetTextFileContent(const std::filesystem::path path)
{
    std::ifstream file(path);
    std::string content;

    if(file)
    {
        // Check file size and allocate space.
        file.seekg(0, std::ios::end);
        content.resize((unsigned int)file.tellg());
        file.seekg(0, std::ios::beg);

        // Read file content.
        file.read(&content[0], content.size());
    }

    return content;
}

std::vector<char> Common::GetBinaryFileContent(const std::filesystem::path path)
{
    std::ifstream file(path, std::ios::binary);
    std::vector<char> content;

    if(file)
    {
        // Check file size and allocate space.
        file.seekg(0, std::ios::end);
        content.resize((unsigned int)file.tellg());
        file.seekg(0, std::ios::beg);

        // Read file content.
        file.read(&content[0], content.size());
    }

    return content;
}

std::string Common::StringLowerCase(const std::string text)
{
    std::string result;

    for(char character : text)
    {
        result.push_back(std::tolower(character));
    }

    return result;
}

std::vector<std::string> Common::StringTokenize(const std::string text, char delimeter)
{
    std::vector<std::string> result;

    auto begin = text.begin();
    auto it = text.begin();

    while(it != text.end())
    {
        if(*it == delimeter)
        {
            result.emplace_back(begin, it);
            begin = ++it;
        }
        else
        {
            ++it;
        }

        if(it == text.end())
        {
            result.emplace_back(begin, it);
        }
    }

    return result;
}

std::string Common::StringTrimLeft(const std::string text, const char* characters)
{
    std::string result = text;
    return result.erase(0, text.find_first_not_of(characters));
}

std::string Common::StringTrimRight(const std::string text, const char* characters)
{
    std::string result = text;
    return result.erase(text.find_last_not_of(characters) + 1);
}

std::string Common::StringTrim(const std::string text, const char* characters)
{
    return StringTrimLeft(StringTrimRight(text, characters), characters);
}

uint32_t Common::CalculateCrc32(uint32_t crc, const uint8_t* data, std::size_t size)
{
    /*
        Implementation based on: https://stackoverflow.com/a/27950866
        It's certain to not be very efficient and could use something faster if needed.
    */

    crc = ~crc;
    while(size--)
    {
        crc ^= *data++;

        for(int i = 0; i < 8; ++i)
        {
            crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
        }
    }

    return ~crc;
}
