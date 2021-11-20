/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Common/Precompiled.hpp"
#include "Common/Logger/Logger.hpp"
#include "Common/Logger/LoggerMessage.hpp"
#include "Common/Logger/LoggerSink.hpp"
#include "Common/Logger/LoggerOutput.hpp"
#include "Common/Logger/LoggerHistory.hpp"
#include "Common/ScopeGuard.hpp"
#include "Common/Profile.hpp"

namespace
{
    Logger::Sink GlobalSink;
    Logger::History GlobalHistory;
    Logger::FileOutput GlobalFileOutput("Log.txt");
    Logger::ConsoleOutput GlobalConsoleOutput;
    Logger::DebuggerOutput GlobalDebuggerOutput;

    Logger::Mode GlobalLoggerMode = Logger::Mode::Normal;
    bool GlobalLoggerInitialized = false;

    void LazyInitialize()
    {
        if(GlobalLoggerInitialized)
            return;

        LOG_PROFILE_SCOPE("Initialize logger");

        // Mark as initialized once done.
        SCOPE_GUARD([]()
        {
            GlobalLoggerInitialized = true;
        });

        // Add default output sinks.
        if(GlobalLoggerMode != Logger::Mode::UnitTests)
        {
            GlobalSink.AddOutput(&GlobalHistory);
            GlobalSink.AddOutput(&GlobalConsoleOutput);
        }

        GlobalSink.AddOutput(&GlobalFileOutput);
        GlobalSink.AddOutput(&GlobalDebuggerOutput);
    }
}

void Logger::Initialize()
{
    LazyInitialize();
}

void Logger::Write(const Logger::Message& message)
{
    LazyInitialize();
    GlobalSink.Write(message);
}

int Logger::AdvanceFrameReference()
{
    LazyInitialize();
    return GlobalSink.AdvanceFrameReference();
}

void Logger::SetMode(Mode mode)
{
    GlobalLoggerMode = mode;
}

Logger::Mode Logger::GetMode()
{
    return GlobalLoggerMode;
}

Logger::Sink& Logger::GetGlobalSink()
{
    LazyInitialize();
    return GlobalSink;
}

Logger::History& Logger::GetGlobalHistory()
{
    LazyInitialize();
    return GlobalHistory;
}

bool Logger::IsInitialized()
{
    return GlobalLoggerInitialized;
}
