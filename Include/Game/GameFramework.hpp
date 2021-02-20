/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Common/Event/Dispatcher.hpp>
#include <Core/ServiceStorage.hpp>
#include "Game/GameState.hpp"

namespace System
{
    class Timer;
    class Window;
    class InputManager;
}

namespace Renderer
{
    class GameRenderer;
}

/*
    Game Framework
*/

namespace Game
{
    class GameState;

    class GameFramework final : private Common::NonCopyable
    {
    public:
        struct CreateFromParams
        {
            const Core::ServiceStorage* services = nullptr;
        };

        enum class CreateErrors
        {
            InvalidArgument,
        };

        using CreateResult = Common::Result<std::unique_ptr<GameFramework>, CreateErrors>;
        static CreateResult Create(const CreateFromParams& params);

        enum class ChangeGameStateErrors
        {
            AlreadyCurrent,
            FailedTransition,
        };

        using ChangeGameStateResult = Common::Result<void, ChangeGameStateErrors>;

        enum class ProcessGameStateResults
        {
            TickedAndUpdated,
            UpdatedOnly,
        };

    public:
        ~GameFramework();

        ChangeGameStateResult ChangeGameState(std::shared_ptr<GameState> gameState);
        ProcessGameStateResults ProcessGameState(float timeDelta);
        bool HasGameState() const;

        struct Events
        {
            // Called whether game state changes.
            // This will be dispatched only on successful game state transition.
            Event::Dispatcher<void(const std::shared_ptr<GameState>&)> gameStateChanged;

            // Called when tick method is called.
            // This does not mean that the state was actually ticked.
            Event::Dispatcher<void()> tickRequested;

            // Called when state had its tick processed.
            // Event can be dispatched multiple times during the same tick method call.
            // This is also good time to run custom tick logic in response.
            Event::Dispatcher<void(float)> tickProcessed;
        } events;

    private:
        GameFramework();

        System::Timer* m_timer = nullptr;
        System::Window* m_window = nullptr;
        Renderer::GameRenderer* m_gameRenderer = nullptr;

        Common::StateMachine<GameState> m_stateMachine;
    };
}
