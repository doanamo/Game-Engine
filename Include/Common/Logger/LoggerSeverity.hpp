/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

/*
    Logger Severity
*/

namespace Logger
{
    struct Severity
    {
        enum Type
        {
            Invalid,

            Debug,
            Profile,
            Info,
            Success,
            Warning,
            Error,
            Fatal,

            Count,
        };
    };

    const char* GetSeverityName(Severity::Type severity);
    const char* GetSeverityMarker(Severity::Type severity);
}
