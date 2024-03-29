/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Graphics/Texture.hpp"
#include "Graphics/RenderContext.hpp"
#include <Core/System/SystemStorage.hpp>
#include <Platform/FileSystem/FileHandle.hpp>
#include <Platform/Image.hpp>
using namespace Graphics;

Texture::Texture() = default;
Texture::~Texture()
{
    if(m_handle != OpenGL::InvalidHandle)
    {
        glDeleteTextures(1, &m_handle);
        OpenGL::CheckErrors();
    }
}

Texture::CreateResult Texture::Create(const CreateFromParams& params)
{
    LOG_PROFILE_SCOPE_FUNC();

    // Validate arguments.
    CHECK_OR_RETURN(params.renderContext != nullptr,
        Common::Failure(CreateErrors::InvalidArgument));
    CHECK_OR_RETURN(params.width > 0 && params.height > 0,
        Common::Failure(CreateErrors::InvalidArgument));
    CHECK_OR_RETURN(params.format != OpenGL::InvalidEnum,
        Common::Failure(CreateErrors::InvalidArgument));

    // Create class instance.
    auto instance = std::unique_ptr<Texture>(new Texture());

    // Create texture handle.
    glGenTextures(1, &instance->m_handle);
    OpenGL::CheckErrors();

    if(instance->m_handle == OpenGL::InvalidHandle)
    {
        LOG_ERROR("Texture could not be created!");
        return Common::Failure(CreateErrors::FailedTextureCreation);
    }

    glBindTexture(GL_TEXTURE_2D, instance->m_handle);
    OpenGL::CheckErrors();

    SCOPE_GUARD([&params]
    {
        glBindTexture(GL_TEXTURE_2D,
            params.renderContext->GetState().GetTextureBinding(GL_TEXTURE_2D));
    });

    if(params.format == GL_RED)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OpenGL::CheckErrors();
    }

    SCOPE_GUARD([&params]
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT,
            params.renderContext->GetState().GetPixelStore(GL_UNPACK_ALIGNMENT));
    });

    glTexImage2D(GL_TEXTURE_2D, 0, params.format, params.width, params.height,
        0, params.format, GL_UNSIGNED_BYTE, params.data);

    if(params.mipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        OpenGL::CheckErrors();
    }

    instance->m_renderContext = params.renderContext;
    instance->m_format = params.format;
    instance->m_width = params.width;
    instance->m_height = params.height;

    return Common::Success(std::move(instance));
}

Texture::CreateResult Texture::Create(Platform::FileHandle& file, const LoadFromFile& params)
{
    LOG_PROFILE_SCOPE_FUNC();
    LOG("Loading texture from \"{}\" file...", file.GetPathString());

    // Validate arguments.
    CHECK_OR_RETURN(params.engineSystems, Common::Failure(CreateErrors::InvalidArgument));

    // Retrieve needed engine systems.
    auto& renderContext = params.engineSystems->Locate<Graphics::RenderContext>();

    // Load image from file.
    auto image = Platform::Image::Create(file, Platform::Image::LoadFromFile()).UnwrapOr(nullptr);
    if(image == nullptr)
    {
        LOG_ERROR("Could not create image from file!");
        return Common::Failure(CreateErrors::FailedImageLoad);
    }
    
    // Determine texture format.
    GLenum textureFormat = GL_NONE;
    switch(image->GetChannels())
    {
    case 1:
        textureFormat = GL_RED;
        break;

    case 2:
        textureFormat = GL_RG;
        break;

    case 3:
        textureFormat = GL_RGB;
        break;

    case 4:
        textureFormat = GL_RGBA;
        break;

    default:
        LOG_ERROR("Unsupported number of image channels!");
        return Common::Failure(CreateErrors::UnsupportedImageFormat);
    }

    // Create texture from image data.
    CreateFromParams createParams;
    createParams.renderContext = &renderContext;
    createParams.width = image->GetWidth();
    createParams.height = image->GetHeight();
    createParams.format = textureFormat;
    createParams.mipmaps = params.mipmaps;
    createParams.data = image->GetData();
    return Create(createParams);
}

void Texture::Update(const void* data)
{
    ASSERT(data != nullptr);

    // Upload new texture data.
    glBindTexture(GL_TEXTURE_2D, m_handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, m_renderContext->GetState().GetTextureBinding(GL_TEXTURE_2D));
    OpenGL::CheckErrors();
}
