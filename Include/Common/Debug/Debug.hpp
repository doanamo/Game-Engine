/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Common/Logger/Logger.hpp>

/*
    Platform Defines
*/

#if defined(PLATFORM_WINDOWS) && defined(CONFIG_DEBUG)
    #define _CRTDBG_MAP_ALLOC
    #define _CRTDBG_MAP_ALLOC_NEW
    #include <crtdbg.h>

    // Override new operator to store additional information about allocations.
    #define CHECKED_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
    #define CHECKED_NEW new
#endif

/*
    Debug Utility
*/

namespace Debug
{
    void Initialize();

    int64_t GetProcessID();
    bool IsDebuggerAttached();
}

/*
    Debug Macros
*/

#define DEBUG_EXPAND_MACRO(x) x
#define DEBUG_STRINGIFY(expression) #expression

#ifdef _MSC_VER
    #define DEBUG_BREAK() __debugbreak()
#else
    #include <signal.h>
    #define DEBUG_BREAK() raise(SIGTRAP)
#endif

#define DEBUG_ABORT() \
    DEBUG_BREAK(); \
    abort()

#ifdef PLATFORM_WINDOWS
    #define DEBUG_BREAK_IF_ATTACHED() \
        if(Debug::IsDebuggerAttached()) \
        { \
            DEBUG_BREAK(); \
        }
#else
    #define DEBUG_BREAK_IF_ATTACHED() ((void)0)
#endif

/*
    Assert Macros

    Ensures that given expression is true in debug configuration.
    Used as sanity check to guard against programming errors.

    Behavior in different build configurations:
    - Debug: Evaluates expression and triggers abort
    - Release: Stripped out completely (does not evaluate expression)

    Example usage:
        ASSERT(m_initialized);
        ASSERT(instance != nullptr, "Invalid instance.");
*/

#ifndef CONFIG_RELEASE
    #define ASSERT_SIMPLE(expression) \
        if(expression) { } else \
        { \
            LOG_FATAL("Assertion failed: " DEBUG_STRINGIFY(expression)); \
            DEBUG_ABORT(); \
        }

    #define ASSERT_MESSAGE(expression, message, ...) \
        if(expression) { } else \
        { \
            LOG_FATAL("Assertion failed: " DEBUG_STRINGIFY(expression) \
                " - " message, ## __VA_ARGS__); \
            DEBUG_ABORT(); \
        }

    #define ASSERT_DEDUCE(arg1, arg2, arg3, arg4, arg5, arg6, arg7, ...) arg7
    #define ASSERT_CHOOSER(...) DEBUG_EXPAND_MACRO(ASSERT_DEDUCE(__VA_ARGS__, ASSERT_MESSAGE, \
        ASSERT_MESSAGE, ASSERT_MESSAGE, ASSERT_MESSAGE, ASSERT_MESSAGE, ASSERT_SIMPLE))
    #define ASSERT(...) DEBUG_EXPAND_MACRO(ASSERT_CHOOSER(__VA_ARGS__)(__VA_ARGS__))
#else
    #define ASSERT(expression, ...) ((void)0)
#endif

/*
    Assert Evaluate Macros

    Ensures that given expression is true in debug configuration.
    In contrary to regular assert, evaluates expression in release configuration.

    Behavior in different build configurations:
    - Debug: Evaluates expression and triggers abort
    - Release: Evaluates expression only (does not trigger abort)

    Example usage:
        ASSERT_EVALUATE(Function());
        ASSERT_EVALUATE(Function() != nullptr, "Invalid call.");
*/

#ifndef CONFIG_RELEASE
    #define ASSERT_EVALUATE_SIMPLE(expression) \
            if(expression) { } else \
            { \
                LOG_FATAL("Assertion failed: " DEBUG_STRINGIFY(expression)); \
                DEBUG_ABORT(); \
            }

    #define ASSERT_EVALUATE_MESSAGE(expression, message, ...) \
            if(expression) { } else \
            { \
                LOG_FATAL("Assertion failed: " DEBUG_STRINGIFY(expression) \
                    " - " message, ## __VA_ARGS__); \
                DEBUG_ABORT(); \
            }

    #define ASSERT_EVALUATE_DEDUCE(arg1, arg2, arg3, arg4, arg5, arg6, arg7, ...) arg7
    #define ASSERT_EVALUATE_CHOOSER(...) DEBUG_EXPAND_MACRO(ASSERT_EVALUATE_DEDUCE(__VA_ARGS__, \
        ASSERT_EVALUATE_MESSAGE, ASSERT_EVALUATE_MESSAGE, ASSERT_EVALUATE_MESSAGE, \
        ASSERT_EVALUATE_MESSAGE, ASSERT_EVALUATE_MESSAGE, ASSERT_EVALUATE_SIMPLE))
    #define ASSERT_EVALUATE(...) DEBUG_EXPAND_MACRO(ASSERT_EVALUATE_CHOOSER(__VA_ARGS__)(__VA_ARGS__))
#else
    #define ASSERT_EVALUATE(expression, ...) (void)(expression)
#endif

/*
    Assert Always Macro

    Ensures that given expression is true in all configurations.
    Used to safeguard against possible errors we choose not to handle.

    Behavior in different build configurations:
    - Debug: Evaluates expression and triggers abort
    - Release: Evaluates expression and triggers abort

    Example usage:
        ASSERT_ALWAYS(m_initialized);
        ASSERT_ALWAYS(instance != nullptr, "Invalid instance.");
*/

#define ASSERT_ALWAYS_SIMPLE(expression) \
    if(expression) { } else \
    { \
        LOG_FATAL("Assertion failed: " DEBUG_STRINGIFY(expression)); \
        DEBUG_ABORT(); \
    }

#define ASSERT_ALWAYS_MESSAGE(expression, message, ...) \
    if(expression) { } else \
    { \
        LOG_FATAL("Assertion failed: " DEBUG_STRINGIFY(expression) \
            " - " message, ## __VA_ARGS__); \
        DEBUG_ABORT(); \
    }

#define ASSERT_ALWAYS_DEDUCE(arg1, arg2, arg3, arg4, arg5, arg6, arg7, ...) arg7
#define ASSERT_ALWAYS_CHOOSER(...) DEBUG_EXPAND_MACRO(ASSERT_ALWAYS_DEDUCE(__VA_ARGS__, \
    ASSERT_ALWAYS_MESSAGE, ASSERT_ALWAYS_MESSAGE, ASSERT_ALWAYS_MESSAGE, \
    ASSERT_ALWAYS_MESSAGE, ASSERT_ALWAYS_MESSAGE, ASSERT_ALWAYS_SIMPLE))
#define ASSERT_ALWAYS(...) DEBUG_EXPAND_MACRO(ASSERT_ALWAYS_CHOOSER(__VA_ARGS__)(__VA_ARGS__))

/*
    Check Macro

    Ensures that given expression is true in debug configuration.
    Used to only print warnings in case we do not want to assert.

    Behavior in different build configurations:
    - Debug: Evaluates expression and logs warning
    - Release: Evaluates expression only when needed (when returning value and not logging)

    Example usage:
        CHECK(m_initialized);
        CHECK(instance != nullptr, "Invalid instance.");
*/

#ifdef CONFIG_DEBUG
    #define CHECK_SIMPLE(expression) \
        if(expression) { } else \
        { \
            LOG_WARNING("Check failed: " DEBUG_STRINGIFY(expression)); \
        }

    #define CHECK_MESSAGE(expression, message, ...) \
        if(expression) { } else \
        { \
            LOG_WARNING("Check failed: " DEBUG_STRINGIFY(expression) \
                " - " message, ## __VA_ARGS__); \
        }

    #define CHECK_OR_RETURN(expression, value, ...) \
        if(expression) { } else \
        { \
            LOG_WARNING("Check failed: " DEBUG_STRINGIFY(expression)); \
            return value; \
        }

    #define CHECK_DEDUCE(arg1, arg2, arg3, arg4, arg5, arg6, arg7, ...) arg7
    #define CHECK_CHOOSER(...) DEBUG_EXPAND_MACRO(CHECK_DEDUCE(__VA_ARGS__, \
        CHECK_MESSAGE, CHECK_MESSAGE, CHECK_MESSAGE, \
        CHECK_MESSAGE, CHECK_MESSAGE, CHECK_SIMPLE))
    #define CHECK(...) DEBUG_EXPAND_MACRO(CHECK_CHOOSER(__VA_ARGS__)(__VA_ARGS__))
#else
    #define CHECK_OR_RETURN(expression, value, ...) \
        if(expression) { } else \
        { \
            return value; \
        }

    #define CHECK(expression, ...) ((void)0)
#endif
