/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

/*
    Sink

    Writes log messages to multiple logging outputs.
*/

namespace Logger
{
    class Output;
    class Message;

    struct SinkContext
    {
        std::string name;
        int referenceFrame = 0;
        int messageIndent = 0;
        bool messageWritten = false;
    };

    class Sink
    {
    public:
        using OutputList = std::vector<Logger::Output*>;

        Sink();
        ~Sink();

        Sink(const Sink&) = delete;
        Sink& operator=(const Sink&) = delete;

        void SetName(std::string name);
        void AddOutput(Logger::Output* output);
        void RemoveOutput(Logger::Output* output);
        void Write(const Logger::Message& message);
        int AdvanceFrameReference();
        void IncreaseIndent();
        void DecreaseIndent();

        const SinkContext& GetContext() const;

    private:
        std::mutex m_lock;
        SinkContext m_context;
        OutputList m_outputs;
    };
}

/*
    Scoped Indent

    Increases logging indent in sink for duration of scope.
*/

namespace Logger
{
    class ScopedIndent
    {
    public:
        ScopedIndent(Sink& sink) :
            m_sink(sink)
        {
            m_sink.IncreaseIndent();
        }

        ~ScopedIndent()
        {
            m_sink.DecreaseIndent();
        }

        ScopedIndent(const ScopedIndent&) = delete;
        ScopedIndent& operator=(const ScopedIndent&) = delete;

    private:
        Sink& m_sink;
    };
}
