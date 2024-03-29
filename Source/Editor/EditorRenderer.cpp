/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Editor/EditorRenderer.hpp"
#include <Core/System/SystemStorage.hpp>
#include <Platform/WindowSystem.hpp>
#include <Platform/Window.hpp>
#include <Core/Resource/ResourceManager.hpp>
using namespace Editor;

namespace
{
    const char* LogAttachFailed = "Failed to attach editor renderer subsystem! {}";
    const char* LogCreateResourcesFailed = "Failed to create editor renderer resources! {}";
}

EditorRenderer::EditorRenderer() = default;
EditorRenderer::~EditorRenderer() = default;

bool EditorRenderer::OnAttach(const EditorSubsystemStorage& editorSubsystems)
{
    // Locate needed engine systems.
    auto& editorContext = editorSubsystems.Locate<EditorSubsystemContext>();
    auto& engineSystems = editorContext.GetEngineSystems();

    m_windowSystem = &engineSystems.Locate<Platform::WindowSystem>();
    m_renderContext = &engineSystems.Locate<Graphics::RenderContext>();

    // Create graphics resources.
    if(!CreateResources(engineSystems))
    {
        LOG_ERROR(LogAttachFailed, "Could not create resources.");
        return false;
    }

    return true;
}

bool EditorRenderer::CreateResources(const Core::EngineSystemStorage& engineSystems)
{
    ASSERT(ImGui::GetCurrentContext() != nullptr, "ImGui context is not set!");

    // Retrieve needed engine systems.
    auto& resourceManager = engineSystems.Locate<Core::ResourceManager>();

    // Vertex buffer.
    Graphics::Buffer::CreateFromParams vertexBufferParams;
    vertexBufferParams.renderContext = m_renderContext;
    vertexBufferParams.usage = GL_STREAM_DRAW;
    vertexBufferParams.elementSize = sizeof(ImDrawVert);

    m_vertexBuffer = Graphics::VertexBuffer::Create(vertexBufferParams).UnwrapOr(nullptr);
    if(m_vertexBuffer == nullptr)
    {
        LOG_ERROR(LogCreateResourcesFailed, "Could not create vertex buffer.");
        return false;
    }

    // Vertex array.
    const Graphics::VertexArray::Attribute inputAttributes[] =
    {
        // Position
        {
            m_vertexBuffer.get(),
            Graphics::VertexArray::AttributeType::Vector2, 
            GL_FLOAT,
            false
        },

        // Texture
        {
            m_vertexBuffer.get(),
            Graphics::VertexArray::AttributeType::Vector2,
            GL_FLOAT,
            false
        },

        // Color
        {
            m_vertexBuffer.get(),
            Graphics::VertexArray::AttributeType::Vector4,
            GL_UNSIGNED_BYTE,
            true
        },
    };

    Graphics::VertexArray::FromArrayParams inputLayoutParams;
    inputLayoutParams.attributeCount = Common::StaticArraySize(inputAttributes);
    inputLayoutParams.attributes = &inputAttributes[0];

    m_vertexArray = Graphics::VertexArray::Create(
        m_renderContext, inputLayoutParams).UnwrapOr(nullptr);
    if(m_vertexArray == nullptr)
    {
        LOG_ERROR(LogCreateResourcesFailed, "Could not create vertex array.");
        return false;
    }

    // Index buffer.
    Graphics::Buffer::CreateFromParams indexBufferParams;
    indexBufferParams.renderContext = m_renderContext;
    indexBufferParams.usage = GL_STREAM_DRAW;
    indexBufferParams.elementSize = sizeof(ImDrawIdx);

    m_indexBuffer = Graphics::IndexBuffer::Create(indexBufferParams).UnwrapOr(nullptr);
    if(m_indexBuffer == nullptr)
    {
        LOG_ERROR(LogCreateResourcesFailed, "Could not create index buffer.");
        return false;
    }

    // Font texture.
    unsigned char* fontData = nullptr;
    int fontWidth = 0;
    int fontHeight = 0;

    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fontData, &fontWidth, &fontHeight);

    if(fontData == nullptr || fontWidth == 0 || fontHeight == 0)
    {
        LOG_ERROR(LogCreateResourcesFailed, "Could not retrieve font data.");
        return false;
    }

    Graphics::Texture::CreateFromParams textureParams;
    textureParams.renderContext = m_renderContext;
    textureParams.width = fontWidth;
    textureParams.height = fontHeight;
    textureParams.format = GL_RGBA;
    textureParams.mipmaps = false;
    textureParams.data = fontData;

    m_fontTexture = Graphics::Texture::Create(textureParams).UnwrapOr(nullptr);
    if(m_fontTexture == nullptr)
    {
        LOG_ERROR(LogCreateResourcesFailed, "Could not create font texture.");
        return false;
    }

    ImGui::GetIO().Fonts->TexID = (void*)(intptr_t)m_fontTexture->GetHandle();

    // Texture sampler.
    Graphics::Sampler::CreateFromParams samplerParams;
    samplerParams.renderContext = m_renderContext;
    samplerParams.textureMinFilter = GL_LINEAR;
    samplerParams.textureMagFilter = GL_LINEAR;

    m_sampler = Graphics::Sampler::Create(samplerParams).UnwrapOr(nullptr);
    if(m_sampler == nullptr)
    {
        LOG_ERROR(LogCreateResourcesFailed, "Could not create texture sampler.");
        return false;
    }

    // Shader.
    Graphics::Shader::LoadFromFile shaderParams;
    shaderParams.renderContext = m_renderContext;

    m_shader = resourceManager.Acquire<Graphics::Shader>(
        "Data/Engine/Shaders/Interface.shader", shaderParams).UnwrapOr(nullptr);

    if(m_shader == nullptr)
    {
        LOG_ERROR(LogCreateResourcesFailed, "Could not create shader.");
        return false;
    }

    return true;
}

void EditorRenderer::OnEndInterface()
{
    ASSERT(ImGui::GetCurrentContext() != nullptr, "ImGui context is not set!");

    // Render interface and retrieve draw data.
    ImGui::Render();

    ImDrawData* drawData = ImGui::GetDrawData();
    if(drawData == nullptr)
        return;

    // Draw vertices from draw data.
    auto& renderState = m_renderContext->PushState();
    SCOPE_GUARD([this]
    {
        m_renderContext->PopState();
    });

    int windowWidth = m_windowSystem->GetWindow().GetWidth();
    int windowHeight = m_windowSystem->GetWindow().GetHeight();
    renderState.Viewport(0, 0, windowWidth, windowHeight);

    renderState.Enable(GL_BLEND);
    renderState.BlendFuncSeparate(
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderState.BlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    renderState.Enable(GL_SCISSOR_TEST);

    m_shader->SetUniform("vertexTransform",
        glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f));
    m_shader->SetUniform("textureDiffuse", 0);
    renderState.UseProgram(m_shader->GetHandle());

    renderState.BindSampler(0, m_sampler->GetHandle());

    ImVec2 position = drawData->DisplayPos;
    for(int list = 0; list < drawData->CmdListsCount; ++list)
    {
        const ImDrawList* commandList = drawData->CmdLists[list];
        const ImDrawIdx* indexBufferOffset = 0;

        m_vertexBuffer->Update(commandList->VtxBuffer.Data, commandList->VtxBuffer.Size);
        m_indexBuffer->Update(commandList->IdxBuffer.Data, commandList->IdxBuffer.Size);

        for(int command = 0; command < commandList->CmdBuffer.Size; ++command)
        {
            const ImDrawCmd* drawCommand = &commandList->CmdBuffer[command];

            if(drawCommand->UserCallback)
            {
                drawCommand->UserCallback(commandList, drawCommand);
            }
            else
            {
                ImVec4 clipRect;
                clipRect.x = drawCommand->ClipRect.x - position.x;
                clipRect.y = drawCommand->ClipRect.y - position.y;
                clipRect.z = drawCommand->ClipRect.z - position.x;
                clipRect.w = drawCommand->ClipRect.w - position.y;

                glm::ivec4 scissorRect;
                scissorRect.x = (int)clipRect.x;
                scissorRect.y = (int)(windowHeight - clipRect.w);
                scissorRect.z = (int)(clipRect.z - clipRect.x);
                scissorRect.w = (int)(clipRect.w - clipRect.y);

                renderState.Scissor(scissorRect.x, scissorRect.y, scissorRect.z, scissorRect.w);
                renderState.ActiveTexture(GL_TEXTURE0);
                renderState.BindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)drawCommand->TextureId);
                renderState.BindVertexArray(m_vertexArray->GetHandle());
                renderState.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer->GetHandle());
                renderState.DrawElements(GL_TRIANGLES,
                    (GLsizei)drawCommand->ElemCount, GL_UNSIGNED_SHORT, indexBufferOffset);
            }

            indexBufferOffset += drawCommand->ElemCount;
        }
    }
}
