/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Common/Precompiled.hpp"
#include "Common/Logger/Logger.hpp"
#include "Common/Logger/Output.hpp"
#include "Common/Logger/Message.hpp"
#include "Common/Logger/Format.hpp"
#include "Common/Logger/Sink.hpp"
using namespace Logger;

FileOutput::FileOutput() = default;

FileOutput::~FileOutput()
{
    // Close file stream.
    if(m_file.is_open())
    {
        // Write session end.
        m_file << DefaultFormat::ComposeSessionEnd();
        m_file.flush();

        // Close file.
        m_file.close();
    }
}

bool FileOutput::Open(std::string filename)
{
    // Check if file stream is already open.
    assert(!m_file.is_open() && "File stream is already open!");

    // Open file stream for writing.
    m_file.open(filename);

    if(!m_file.is_open())
    {
        LOG_ERROR("Log file output could not be opened!");
        return false;
    }

    // Write session start.
    m_file << DefaultFormat::ComposeSessionStart();
    m_file.flush();

    // Success!
    return true;
}

void FileOutput::Write(const Message& message, const SinkContext& context)
{
    assert(m_file.is_open() && "File stream is not open!");

    // Write log message.
    m_file << DefaultFormat::ComposeMessage(message, context);
    m_file.flush();
}

ConsoleOutput::ConsoleOutput()
{
    // Write session start.
    std::cout << DefaultFormat::ComposeSessionStart();
}

ConsoleOutput::~ConsoleOutput()
{
    // Write session end.
    std::cout << DefaultFormat::ComposeSessionEnd();
}

void ConsoleOutput::Write(const Message& message, const SinkContext& context)
{
    // Write log message.
    std::cout << DefaultFormat::ComposeMessage(message, context);
}

DebuggerOutput::DebuggerOutput() = default;
DebuggerOutput::~DebuggerOutput() = default;

void DebuggerOutput::Write(const Message& message, const SinkContext& context)
{
#ifdef WIN32
    // Check if debugger is attached.
    if(!IsDebuggerPresent())
        return;

    // Write log message.
    std::string output = DefaultFormat::ComposeMessage(message, context);
    OutputDebugStringA(output.c_str());
#endif
}