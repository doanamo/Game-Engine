/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Common/Events/EventBroker.hpp>
#include "Platform/InputDefinitions.hpp"

/*
    Input State
*/

namespace Platform
{
    class InputManager;

    class InputState final : private Common::NonCopyable
    {
    public:
        friend InputManager;

        ~InputState();

        bool IsKeyboardKeyPressed(KeyboardKeys::Type key, bool repeat = true) const;
        bool IsKeyboardKeyReleased(KeyboardKeys::Type key, bool repeat = true) const;
        bool IsMouseButtonPressed(KeyboardKeys::Type key, bool repeat = true) const;
        bool IsMouseButtonReleased(KeyboardKeys::Type key, bool repeat = true) const;

    public:
        Event::Broker events;

    private:
        InputState();

        void UpdateStates(float timeDelta);
        void UpdateStateTimes(float timeDelta);
        void UpdateStateTransitions();
        void ResetStates();

        bool OnTextInput(const InputEvents::TextInput& event);
        bool OnKeyboardKey(const InputEvents::KeyboardKey& event);
        bool OnMouseButton(const InputEvents::MouseButton& event);
        bool OnMouseScroll(const InputEvents::MouseScroll& event);
        void OnCursorPosition(const InputEvents::CursorPosition& event);
        void OnCursorEnter(const InputEvents::CursorEnter& event);

    private:
        InputEvents::KeyboardKey m_keyboardKeyStates[KeyboardKeys::Count];
        InputEvents::MouseButton m_mouseButtonStates[MouseButtons::Count];
    };
}
