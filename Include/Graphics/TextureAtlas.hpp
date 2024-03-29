/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <Core/System/EngineSystem.hpp>

namespace Platform
{
    class FileHandle;
}

/*
    Texture Atlas

    Stores multiple images that can be referenced by name in a single texture.
*/

namespace Graphics
{
    class RenderContext;
    class Texture;
    class TextureView;

    class TextureAtlas final : private Common::NonCopyable
    {
        REFLECTION_ENABLE(TextureAtlas)

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

        using CreateResult = Common::Result<std::unique_ptr<TextureAtlas>, CreateErrors>;

        static CreateResult Create();
        static CreateResult Create(Platform::FileHandle& file, const LoadFromFile& params);

        using ConstTexturePtr = std::shared_ptr<const Texture>;
        using RegionMap = std::unordered_map<std::string, glm::ivec4>;

    public:
        ~TextureAtlas();

        bool AddRegion(std::string name, glm::ivec4 imageCoords);
        TextureView GetRegion(std::string name);

    private:
        TextureAtlas();

    private:
        ConstTexturePtr m_texture;
        RegionMap m_regions;
    };
};

REFLECTION_TYPE(Graphics::TextureAtlas)
