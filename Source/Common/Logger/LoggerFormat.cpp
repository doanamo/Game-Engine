/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Common/Logger/LoggerFormat.hpp"
#include "Common/Logger/LoggerMessage.hpp"
#include "Common/Logger/LoggerSink.hpp"
using namespace Logger;

namespace
{
    bool CaseInsensitiveCharacterComparison(char a, char b)
    {
        return std::tolower(a) == std::tolower(b);
    };
}

std::string DefaultFormat::ComposeSessionStart()
{
    // Format session start text.
    std::tm time = fmt::localtime(std::time(nullptr));
    std::string sessionText = fmt::format("Session started at {:%Y-%m-%d %H:%M:%S}\n\n", time);

    // Format log message legend text.
    sessionText += fmt::format(
        "Log message legend: "
        "[{}] Debug, [{}] Profile, [{}] Info, [{}] Success, [{}] Warning, [{}] Error, [{}] Fatal\n",
        GetSeverityMarker(Severity::Debug),
        GetSeverityMarker(Severity::Profile),
        GetSeverityMarker(Severity::Info),
        GetSeverityMarker(Severity::Success),
        GetSeverityMarker(Severity::Warning),
        GetSeverityMarker(Severity::Error),
        GetSeverityMarker(Severity::Fatal)
    );

    // Format log message format text.
    sessionText += fmt::format(
        "Log message format: [Time][Frame][Type] Message {{Source:Line}}\n\n");

    return sessionText;
}

std::string DefaultFormat::ComposeMessage(const Message& message, const SinkContext& context)
{
    // Retrieve current system time.
    // fmt::localtime() is a thread safe version of std::localtime().
    std::tm time = fmt::localtime(std::time(nullptr));

    // Format log message.
    std::string messageText;
    messageText.reserve(256);

    messageText += fmt::format("[{:%H:%M:%S}]", time);
    messageText += fmt::format("[{:03d}]", context.referenceFrame % 1000);
    messageText += fmt::format("[{}]", GetSeverityMarker(message.GetSeverity()));
    messageText += fmt::format(" {: >{}}{}", "", context.messageIndent, message.GetText());
        
    // Format message source.
    if(message.GetSource())
    {
        std::string sourcePath = message.GetSource();

        // Normalize source path delimiters.
        std::replace(sourcePath.begin(), sourcePath.end(), '\\', '/');

        // Find and remove base path to include directory.
        std::string includeDir = "Include/";

        auto reverseIt = std::search(
            sourcePath.rbegin(), sourcePath.rend(),
            includeDir.rbegin(), includeDir.rend(),
            CaseInsensitiveCharacterComparison
        );

        if(reverseIt != sourcePath.rend())
        {
            reverseIt += includeDir.length();
        }

        // Find and remove base path to source directory.
        if(reverseIt == sourcePath.rend())
        {
            std::string sourceDir = "Source/";

            reverseIt = std::search(
                sourcePath.rbegin(), sourcePath.rend(),
                sourceDir.rbegin(), sourceDir.rend(),
                CaseInsensitiveCharacterComparison
            );

            if(reverseIt != sourcePath.rend())
            {
                reverseIt += sourceDir.length();
            }
        }

        // Remove base path to file.
        if(reverseIt != sourcePath.rend())
        {
            sourcePath.erase(sourcePath.begin(), reverseIt.base());
        }

        // Format source path.
        messageText += fmt::format(" {{{}:{}}}", sourcePath, message.GetLine());
    }

    // Write message suffix.
    messageText += '\n';
    return messageText;
}

std::string DefaultFormat::ComposeSessionEnd()
{
    // Format session end string.
    std::string sessionText;
    std::tm time = fmt::localtime(std::time(nullptr));
    sessionText += fmt::format("\nSession ended at {:%Y-%m-%d %H:%M:%S}\n\n", time);
    return sessionText;
}
