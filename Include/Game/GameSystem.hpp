/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Core/System/SystemInterface.hpp>

/*
    Game System

    Base class for game systems to be used with system storage.
*/

namespace Game
{
    class GameSystem : public Core::SystemInterface<GameSystem>
    {
        REFLECTION_ENABLE(GameSystem)

    public:
        virtual ~GameSystem() = default;
        virtual void OnTick(float timeDelta) {}

    protected:
        GameSystem() = default;
    };

    using GameSystemStorage = Core::SystemStorage<GameSystem>;
}

REFLECTION_TYPE(Game::GameSystem)
