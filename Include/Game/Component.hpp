/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include "Game/EntityHandle.hpp"

/*
    Component

    Base class for component types.
*/

namespace Game
{
    class ComponentSystem;

    class Component
    {
        REFLECTION_ENABLE(Component)

    protected:
        Component() = default;
        virtual ~Component() = default;

    public:
        virtual bool OnInitialize(ComponentSystem* componentSystem, const EntityHandle& entitySelf)
        {
            // Return true to indicate that component's initialization succeeded.
            return true;
        }
    };
}

REFLECTION_TYPE(Game::Component)
