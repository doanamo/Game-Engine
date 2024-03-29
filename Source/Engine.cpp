/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Engine.hpp"
#include <Reflection/Reflection.hpp>
#include <Core/Build/Build.hpp>
#include <Core/System/SystemStorage.hpp>
#include <Core/System/EngineMetrics.hpp>
#include <Core/System/FrameRateLimiter.hpp>
#include <Core/Script/ScriptSystem.hpp>
#include <Core/Config/ConfigSystem.hpp>
#include <Platform/PlatformSystem.hpp>
#include <Platform/TimerSystem.hpp>
#include <Platform/FileSystem/FileSystem.hpp>
#include <Core/Resource/ResourceManager.hpp>
#include <Platform/InputManager.hpp>
#include <Platform/WindowSystem.hpp>
#include <Graphics/RenderContext.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Sprite/SpriteRenderer.hpp>
#include <Renderer/GameRenderer.hpp>
#include <Game/GameFramework.hpp>
#include <Editor/EditorSystem.hpp>
using namespace Engine;

namespace
{
    const char* LogCreateEngineFailed = "Failed to create engine! {}";
    const char* LogCreateSystemsFailed = "Failed to create engine systems! {}";
    const char* LogLoadDefaultResourcesFailed = "Failed to load default resources! {}";
}

Root::Root() = default;
Root::~Root() = default;

Root::CreateResult Root::Create(const ConfigVariables& configVars)
{
    /*
        Create engine instance and return it if initialization succeeds.
        First global systems are initialized for various debug facilities.
        Then engine and its systems are created so game state can be hosted.
        At the end we load default resources such as placeholder texture.
    */

    LOG_PROFILE_SCOPE_FUNC();

    // Initialize static systems.
    Debug::Initialize();
    Logger::Initialize();
    Build::Initialize();
    Reflection::Initialize();

    // Create engine instance.
    auto engine = std::unique_ptr<Root>(new Root());

    if(auto failureResult = engine->CreateSystems(configVars).AsFailure())
    {
        LOG_FATAL(LogCreateEngineFailed, "Could not create engine systems.");
        return Common::Failure(failureResult.Unwrap());
    }

    if(auto failureResult = engine->LoadDefaultResources().AsFailure())
    {
        LOG_FATAL(LogCreateEngineFailed, "Could not load default resources.");
        return Common::Failure(failureResult.Unwrap());
    }

    return Common::Success(std::move(engine));
}

Common::Result<void, Root::CreateErrors> Root::CreateSystems(const ConfigVariables& configVars)
{
    LOG_PROFILE_SCOPE_FUNC();

    // Setup storage context for initialization.
    m_engineSystems.GetContext().initialConfigVars = configVars;

    SCOPE_GUARD([this]
    {
        Common::FreeContainer(m_engineSystems.GetContext().initialConfigVars);
    });

    // Create default systems.
    const std::vector<Reflection::TypeIdentifier> defaultEngineSystemTypes =
    {
        Reflection::GetIdentifier<Core::ScriptSystem>(),
        Reflection::GetIdentifier<Core::ConfigSystem>(),
        Reflection::GetIdentifier<Core::EngineMetrics>(),
        Reflection::GetIdentifier<Core::FrameRateLimiter>(),
        Reflection::GetIdentifier<Platform::PlatformSystem>(),
        Reflection::GetIdentifier<Platform::FileSystem>(),
        Reflection::GetIdentifier<Platform::TimerSystem>(),
        Reflection::GetIdentifier<Platform::WindowSystem>(),
        Reflection::GetIdentifier<Platform::InputManager>(),
        Reflection::GetIdentifier<Core::ResourceManager>(),
        Reflection::GetIdentifier<Game::GameFramework>(),
        Reflection::GetIdentifier<Graphics::RenderContext>(),
        Reflection::GetIdentifier<Graphics::SpriteRenderer>(),
        Reflection::GetIdentifier<Renderer::GameRenderer>(),
        Reflection::GetIdentifier<Editor::EditorSystem>(),
    };

    if(!m_engineSystems.CreateFromTypes(defaultEngineSystemTypes))
    {
        LOG_ERROR(LogCreateSystemsFailed, "Could not populate system storage.");
        return Common::Failure(CreateErrors::FailedSystemCreation);
    }

    if(!m_engineSystems.Finalize())
    {
        LOG_ERROR(LogCreateSystemsFailed, "Could not finalize system storage.");
        return Common::Failure(CreateErrors::FailedSystemCreation);
    }

    return Common::Success();
}

Common::Result<void, Root::CreateErrors> Root::LoadDefaultResources()
{
    LOG_PROFILE_SCOPE_FUNC();

    // Locate systems needed to load resources.
    auto& fileSystem = m_engineSystems.Locate<Platform::FileSystem>();
    auto& resourceManager = m_engineSystems.Locate<Core::ResourceManager>();

    // Default texture placeholder for when requested texture is missing.
    // Texture is made to be easily spotted to indicate potential issues.
    const std::unique_ptr<Platform::FileHandle> defaultTextureFileResult = fileSystem.OpenFile(
        "Data/Engine/Default/Texture.png", Platform::FileHandle::OpenFlags::Read)
        .UnwrapOr(nullptr);

    if(defaultTextureFileResult != nullptr)
    {
        Graphics::Texture::LoadFromFile defaultTextureParams;
        defaultTextureParams.engineSystems = &m_engineSystems;

        if(auto defaultTextureResult = Graphics::Texture::Create(
            *defaultTextureFileResult, defaultTextureParams))
        {
            resourceManager.SetDefault<Graphics::Texture>(defaultTextureResult.Unwrap());
        }
        else
        {
            LOG_ERROR(LogLoadDefaultResourcesFailed, "Could not load default texture resource.");
            return Common::Failure(CreateErrors::FailedResourceLoading);
        }
    }
    else
    {
        LOG_ERROR(LogLoadDefaultResourcesFailed, "Could not resolve default texture path.");
        return Common::Failure(CreateErrors::FailedResourceLoading);
    }

    return Common::Success();
}

void Root::ProcessFrame()
{
    // Before processing frame.
    Logger::AdvanceFrameReference();

    m_engineSystems.ForEach([](Core::EngineSystem& engineSystem)
    {
        engineSystem.OnPreFrame();
        return true;
    });

    // Begin processing frame.
    m_engineSystems.ForEach([](Core::EngineSystem& engineSystem)
    {
        engineSystem.OnBeginFrame();
        return true;
    });

    // Perform frame processing.
    m_engineSystems.ForEach([](Core::EngineSystem& engineSystem)
    {
        engineSystem.OnProcessFrame();
        return true;
    });

    // End processing frame.
    m_engineSystems.ForEachReverse([](Core::EngineSystem& engineSystem)
    {
        engineSystem.OnEndFrame();
        return true;
    });

    // After processing frame.
    m_engineSystems.ForEach([](Core::EngineSystem& engineSystem)
    {
        engineSystem.OnPostFrame();
        return true;
    });
}

Root::ErrorCode Root::Run()
{
    /*
        Initiates main loop that exits only when application requests to be closed. Before main
        loop is run we have to set window context as current, then timer is reset on the first
        iteration to exclude time accumulated during initialization.
    */

    m_engineSystems.ForEach([](Core::EngineSystem& engineSystem)
    {
        engineSystem.OnRunEngine();
        return true;
    });

#ifndef PLATFORM_EMSCRIPTEN
    while(true)
    {
        bool requestingExit = false;
        m_engineSystems.ForEach([&requestingExit](Core::EngineSystem& engineSystem)
        {
            requestingExit |= engineSystem.IsRequestingExit();
            return true;
        });

        if(requestingExit)
            break;

        ProcessFrame();
    }
#else
    auto mainLoopIteration = [](void* engine)
    {
        ASSERT(engine != nullptr);
        Root* root = static_cast<Root*>(engine);
        root->ProcessFrame();
    };

    emscripten_set_main_loop_arg(mainLoopIteration, this, 0, 1);
#endif

    return ErrorCode(0);
}
