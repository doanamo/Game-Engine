/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Game/Systems/InterpolationSystem.hpp"
#include "Game/Components/TransformComponent.hpp"
#include "Game/Components/SpriteAnimationComponent.hpp"
#include "Game/ComponentSystem.hpp"
#include "Game/GameInstance.hpp"
using namespace Game;

InterpolationSystem::InterpolationSystem() = default;
InterpolationSystem::~InterpolationSystem() = default;

bool InterpolationSystem::OnAttach(const GameSystemStorage& gameSystems)
{
    // Retrieve needed game systems.
    m_componentSystem = &gameSystems.Locate<ComponentSystem>();

    return true;
}

void InterpolationSystem::OnTick(float timeDelta)
{
    // Reset interpolation state of all sprite transform components.
    for(auto& transformComponent : m_componentSystem->GetPool<Game::TransformComponent>())
    {
        transformComponent.ResetInterpolation();
    }

    // Reset interpolation states of all sprite animation components.
    for(auto& spriteAnimationComponent : m_componentSystem->GetPool<Game::SpriteAnimationComponent>())
    {
        spriteAnimationComponent.ResetInterpolation();
    }
}
