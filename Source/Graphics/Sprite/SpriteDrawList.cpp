/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Graphics/Sprite/SpriteDrawList.hpp"
using namespace Graphics;

SpriteDrawList::SpriteDrawList() = default;
SpriteDrawList::~SpriteDrawList() = default;

void SpriteDrawList::ReserveSprites(std::size_t count)
{
    m_spriteInfo.reserve(count);
    m_spriteData.reserve(count);
    m_spriteSort.reserve(count);
}

void SpriteDrawList::AddSprite(const Sprite& sprite)
{
    m_spriteInfo.push_back(sprite.info);
    m_spriteData.push_back(sprite.data);
}

void SpriteDrawList::ClearSprites()
{
    m_spriteInfo.clear();
    m_spriteData.clear();
    m_spriteSort.clear();
}

void SpriteDrawList::SortSprites()
{
    ASSERT(m_spriteInfo.size() == m_spriteData.size(),
        "Arrays of sprite info and data have different size!");

    // Define sorting function.
    // Assumes typical orthogonal 2D projection.
    const std::vector<Sprite::Info>& spriteInfo = m_spriteInfo;
    const std::vector<Sprite::Data>& spriteData = m_spriteData;

    auto SpriteSort = [&spriteInfo, &spriteData]
        (const std::size_t& a, const std::size_t& b) -> bool
    {
        // Get sprite info and data.
        const auto& spriteInfoA = spriteInfo[a];
        const auto& spriteInfoB = spriteInfo[b];

        // Sort by transparency (opaque before transparent).
        if(spriteInfoA.transparent < spriteInfoB.transparent)
            return true;

        // Sort by texture (group same textures).
        if(spriteInfoA.texture < spriteInfoB.texture)
            return true;

        // Sort by filtered (group same filters)
        if(spriteInfoA.filtered < spriteInfoB.filtered)
            return true;

        return false;
    };

    // Generate sprite indices.
    m_spriteSort.resize(m_spriteInfo.size());
    std::iota(m_spriteSort.begin(), m_spriteSort.end(), 0);

    // Create sort permutation for the most efficient drawing.
    // We use stable sort to not introduce possible flickering in rendered images.
    std::stable_sort(m_spriteSort.begin(), m_spriteSort.end(), SpriteSort);

    // Reorder sprite info and data arrays to match sorted indices.
    bool reorderResult = true;
    reorderResult &= Common::ReorderWithIndices(m_spriteInfo, m_spriteSort);
    reorderResult &= Common::ReorderWithIndices(m_spriteData, m_spriteSort);
    ASSERT(reorderResult, "Reorder with indices failed!");
}
