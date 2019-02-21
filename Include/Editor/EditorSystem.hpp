/*
    Copyright (c) 2018-2019 Piotr Doan. All rights reserved.
*/

#pragma once

#include "Event/Receiver.hpp"
#include "System/InputDefinitions.hpp"
#include "Editor/EditorRenderer.hpp"
#include "Editor/EditorShell.hpp"

// Forward declarations.
namespace Engine
{
    class Root;
}

namespace System
{
    class ResourceManager;
};

namespace Game
{
    class GameState;
};

/*
    Editor System

    Displays and handles different editor interfaces.
*/

namespace Editor
{
    // Editor system class.
    class EditorSystem
    {
    public:
        EditorSystem();
        ~EditorSystem();

        // Disallow copying.
        EditorSystem(const EditorSystem& other) = delete;
        EditorSystem& operator=(const EditorSystem& other) = delete;

        // Move constructor and assignment.
        EditorSystem(EditorSystem&& other);
        EditorSystem& operator=(EditorSystem&& other);

        // Initializes the editor system.
        bool Initialize(Engine::Root* engine);

        // Updates the editor interface.
        void Update(float timeDelta);

        // Draws the editor interface.
        void Draw();

    private:
        // Callback function for cursor position events.
        void CursorPositionCallback(const System::InputEvents::CursorPosition& event);

        // Callback function for mouse button events.
        bool MouseButtonCallback(const System::InputEvents::MouseButton& event);

        // Callback function for mouse scroll events.
        bool MouseScrollCallback(const System::InputEvents::MouseScroll& event);

        // Callback function for keyboard key events.
        bool KeyboardKeyCallback(const System::InputEvents::KeyboardKey& event);

        // Callback function for text input events.
        bool TextInputCallback(const System::InputEvents::TextInput& event);

    private:
        // Destroys the user interface context.
        void DestroyContext();

    private:
        // Editor reference.
        Engine::Root* m_engine;

        // User interface context.
        ImGuiContext* m_interface;

        // Window event callbacks.
        Event::Receiver<void(const System::InputEvents::CursorPosition&)> m_receiverCursorPosition;
        Event::Receiver<bool(const System::InputEvents::MouseButton&)> m_receiverMouseButton;
        Event::Receiver<bool(const System::InputEvents::MouseScroll&)> m_receiverMouseScroll;
        Event::Receiver<bool(const System::InputEvents::KeyboardKey&)> m_receiverKeyboardKey;
        Event::Receiver<bool(const System::InputEvents::TextInput&)> m_receiverTextInput;

        // Editor systems.
        EditorRenderer m_editorRenderer;
        EditorShell m_editorShell;

        // Initialization state.
        bool m_initialized;
    };
}
