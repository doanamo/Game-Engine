/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Common/Logger/Logger.hpp"
#include "Common/Logger/LoggerOutput.hpp"
#include "Common/Logger/LoggerMessage.hpp"
#include "Common/Logger/LoggerFormat.hpp"
#include "Common/Logger/LoggerSink.hpp"
using namespace Logger;

/*
    File Output
*/

FileOutput::FileOutput(std::string filename)
{
    assert(!m_file.is_open() && "File stream is already open!");

    m_file.open(filename);
    if(m_file.is_open())
    {
        m_file << DefaultFormat::ComposeSessionStart();
        m_file.flush();
    }
}

FileOutput::~FileOutput()
{
    if(m_file.is_open())
    {
        m_file << DefaultFormat::ComposeSessionEnd();
        m_file.flush();
        m_file.close();
    }
}

bool FileOutput::Initialize() const
{
    return m_file.is_open();
}

void FileOutput::Write(const Message& message, const SinkContext& context)
{
    assert(m_file.is_open() && "File stream is not open!");

    m_file << DefaultFormat::ComposeMessage(message, context);
    m_file.flush();
}

/*
    Console Output
*/

ConsoleOutput::ConsoleOutput() = default;
ConsoleOutput::~ConsoleOutput() = default;

bool ConsoleOutput::Initialize() const
{
#if defined(PLATFORM_WINDOWS)
    return GetConsoleWindow();
#elif defined(PLATFORM_LINUX)
    return isatty(STDOUT_FILENO);
#else
    return true;
#endif
}

void ConsoleOutput::Write(const Message& message, const SinkContext& context)
{
    fmt::print("{}", DefaultFormat::ComposeMessage(message, context));
}

/*
    Debugger Output
*/

DebuggerOutput::DebuggerOutput() = default;
DebuggerOutput::~DebuggerOutput() = default;

bool DebuggerOutput::Initialize() const
{
#ifdef WIN32
    return IsDebuggerPresent();
#else
    return false;
#endif
}

void DebuggerOutput::Write(const Message& message, const SinkContext& context)
{
#ifdef WIN32
    std::string output = DefaultFormat::ComposeMessage(message, context);
    OutputDebugStringA(output.c_str());
#endif
}
