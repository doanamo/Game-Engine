/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Core/System/EngineSystem.hpp>
#include "Graphics/RenderState.hpp"

namespace Platform
{
    class WindowSystem;
}

/*
    Render Context

    Manages internal state of rendering system.
*/

namespace Graphics
{
    class RenderContext final : public Core::EngineSystem
    {
        REFLECTION_ENABLE(RenderContext, Core::EngineSystem)

    public:
        RenderContext();
        ~RenderContext() override;

        void MakeCurrent();
        RenderState& PushState();
        void PopState();

        RenderState& GetState()
        {
            return m_currentState;
        }

    private:
        bool OnAttach(const Core::EngineSystemStorage& engineSystems) override;

    private:
        Platform::WindowSystem* m_windowSystem = nullptr;

        RenderState m_currentState;
        std::stack<RenderState> m_pushedStates;
    };
}

REFLECTION_TYPE(Graphics::RenderContext, Core::EngineSystem)
