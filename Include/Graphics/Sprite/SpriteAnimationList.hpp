/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Core/System/EngineSystem.hpp>
#include "Graphics/TextureView.hpp"

namespace Platform
{
    class FileHandle;
}

/*
    Animation List
*/

namespace Graphics
{
    class RenderContext;

    class SpriteAnimationList final : private Common::NonCopyable
    {
        REFLECTION_ENABLE(SpriteAnimationList)

    public:
        struct LoadFromFile
        {
            const Core::EngineSystemStorage* engineSystems = nullptr;
        };

        enum class CreateErrors
        {
            InvalidArgument,
            FailedResourceLoading,
        };

        using CreateResult = Common::Result<std::unique_ptr<SpriteAnimationList>, CreateErrors>;

        static CreateResult Create();
        static CreateResult Create(Platform::FileHandle& file, const LoadFromFile& params);

        struct Frame
        {
            Frame();
            Frame(TextureView&& textureView, float duration);

            TextureView textureView;
            float duration = 0.0f;
        };

        struct Animation
        {
            Frame GetFrameByTime(float animationTime) const;

            std::vector<Frame> frames;
            float duration = 0.0f;
        };

        using AnimationList = std::vector<Animation>;
        using AnimationMap = std::unordered_map<std::string, std::uint32_t>;

        using AnimationIndexResult = Common::Result<uint32_t, void>;

    public:
        ~SpriteAnimationList();

        AnimationIndexResult GetAnimationIndex(std::string animationName) const;
        const Animation* GetAnimationByIndex(std::size_t animationIndex) const;

    private:
        SpriteAnimationList();

    private:
        AnimationList m_animationList;
        AnimationMap m_animationMap;
    };
}

REFLECTION_TYPE(Graphics::SpriteAnimationList)
