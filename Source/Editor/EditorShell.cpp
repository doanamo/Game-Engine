/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Editor/EditorShell.hpp"
#include "Editor/Modules/InputManagerEditor.hpp"
#include "Editor/Modules/GameInstanceEditor.hpp"
#include <Core/System/EngineMetrics.hpp>
#include <Platform/WindowSystem.hpp>
#include <Platform/Window.hpp>
using namespace Editor;

namespace
{
    const char* LogAttachFailed = "Failed to create editor shell subsystem! {}";
    const char* LogCreateModulesFailed = "Failed to create editor shell modules! {}";
}

EditorShell::EditorShell() = default;
EditorShell::~EditorShell() = default;

bool EditorShell::OnAttach(const EditorSubsystemStorage& editorSubsystems)
{
    // Locate needed engine systems.
    auto& editorContext = editorSubsystems.Locate<EditorSubsystemContext>();
    auto& engineSystems = editorContext.GetEngineSystems();

    m_engineMetrics = &engineSystems.Locate<Core::EngineMetrics>();
    m_windowSystem = &engineSystems.Locate<Platform::WindowSystem>();

    // Create editor modules.
    if(!CreateModules(engineSystems))
    {
        LOG_ERROR(LogAttachFailed, "Could not create editor modules.");
        return false;
    }

    return true;
}

bool EditorShell::CreateModules(const Core::EngineSystemStorage& engineSystems)
{
    // Create editor module context.
    auto editorModuleContext = std::make_unique<EditorModuleContext>(engineSystems);
    if(!editorModuleContext || !m_editorModules.Attach(std::move(editorModuleContext)))
    {
        LOG_ERROR(LogCreateModulesFailed, "Could not attach editor module context.");
        return false;
    }

    // Create default editor modules.
    const std::vector<Reflection::TypeIdentifier> defaultEditorModuleTypes =
    {
        Reflection::GetIdentifier<InputManagerEditor>(),
        Reflection::GetIdentifier<GameInstanceEditor>(),
    };

    if(!m_editorModules.CreateFromTypes(defaultEditorModuleTypes))
    {
        LOG_ERROR(LogCreateModulesFailed, "Could not populate system storage.");
        return false;
    }

    if(!m_editorModules.Finalize())
    {
        LOG_ERROR(LogCreateModulesFailed, "Could not finalize system storage.");
        return false;
    }

    return true;
}

void EditorShell::OnBeginInterface(float timeDelta)
{
    if(m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    DisplayMenuBar();
    DisplayFramerate();

    m_editorModules.ForEach([timeDelta](EditorModule& editorModule)
    {
        editorModule.OnDisplay(timeDelta);
        return true;
    });
}

void EditorShell::DisplayMenuBar()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("Editor"))
        {
            ImGui::MenuItem("Show Demo", "", &m_showDemoWindow, true);
            ImGui::Separator();

            if(ImGui::MenuItem("Exit"))
            {
                m_windowSystem->GetWindow().Close();
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        m_editorModules.ForEach([](EditorModule& editorModule)
        {
            editorModule.OnDisplayMenuBar();
            return true;
        });

        ImGui::EndMainMenuBar();
    }
}

void EditorShell::DisplayFramerate()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));

    ImGui::SetNextWindowPos(ImVec2(4.0f, m_windowSystem->GetWindow().GetHeight() - 4.0f),
        ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_AlwaysAutoResize;
    flags |= ImGuiWindowFlags_NoSavedSettings;
    flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    flags |= ImGuiWindowFlags_NoNav;

    if(ImGui::Begin("Framerate Counter Button", nullptr, flags))
    {
        std::string text = fmt::format("FPS: {:.0f} ({:.3f} ms)",
            static_cast<float>(m_engineMetrics->GetFrameRateAverage()),
            static_cast<float>(m_engineMetrics->GetFrameTimeAverage() * 1000.0));
        if(ImGui::Button(text.c_str()))
        {
        }
    }
    ImGui::End();

    ImGui::PopStyleVar(4);
}
