/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Core/System/SystemStorage.hpp>
#include <Core/System/EngineSystem.hpp>
#include "Editor/EditorSubsystem.hpp"
#include "Editor/EditorModule.hpp"

namespace Core
{
    class EngineMetrics;
};

namespace Platform
{
    class WindowSystem;
};

/*
    Editor Shell

    Main front end class for editor interface.
*/

namespace Editor
{
    class EditorShell final : private EditorSubsystem
    {
        REFLECTION_ENABLE(EditorShell, EditorSubsystem)

    public:
        EditorShell();
        ~EditorShell();

    private:
        bool OnAttach(const EditorSubsystemStorage& editorSubsystems) override;
        bool CreateModules(const Core::EngineSystemStorage& engineSystems);

        void OnBeginInterface(float timeDelta) override;
        void DisplayMenuBar();
        void DisplayFramerate();

    private:
        Core::EngineMetrics* m_engineMetrics = nullptr;
        Platform::WindowSystem* m_windowSystem = nullptr;

        EditorModuleStorage m_editorModules;

        bool m_showDemoWindow = false;
    };
}

REFLECTION_TYPE(Editor::EditorShell, Editor::EditorSubsystem)
