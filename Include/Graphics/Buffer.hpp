/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include "Graphics/RenderState.hpp"

/*
    Buffer
    
    Generic buffer base class that can handle different types of OpenGL buffers.
    Supported buffer types include vertex buffer, index buffer and instance buffer.
*/

namespace Graphics
{
    class RenderContext;

    class Buffer : private Common::NonCopyable
    {
    public:
        struct CreateFromParams
        {
            Graphics::RenderContext* renderContext = nullptr;
            GLenum usage = GL_STATIC_DRAW;
            std::size_t elementSize = 0;
            std::size_t elementCount = 0;
            const void* data = nullptr;
        };

        enum class BufferErrors
        {
            InvalidArgument,
            FailedResourceCreation,
        };

        using BufferResult = Common::Result<void, BufferErrors>;

    public:
        void Update(const void* data, std::size_t elementCount);

        GLenum GetType() const
        {
            return m_type;
        }

        GLuint GetHandle() const
        {
            return m_handle;
        }

        std::size_t GetElementSize() const
        {
            return m_elementSize;
        }

        std::size_t GetElementCount() const
        {
            return m_elementCount;
        }

        virtual GLenum GetElementType() const
        {
            return OpenGL::InvalidEnum;
        }

        virtual bool IsInstanced() const
        {
            return false;
        }

    protected:
        Buffer();
        virtual ~Buffer();

        BufferResult Initialize(GLenum type, const CreateFromParams& params);

    protected:
        RenderContext* m_renderContext = nullptr;

        GLuint m_handle = OpenGL::InvalidHandle;
        GLenum m_type = OpenGL::InvalidEnum;
        GLenum m_usage = OpenGL::InvalidEnum;

        std::size_t m_elementSize = 0;
        std::size_t m_elementCount = 0;
    };
}

/*
    Vertex Buffer
*/

namespace Graphics
{
    class VertexBuffer final : public Buffer
    {
    public:
        using BufferResult = Common::Result<std::unique_ptr<VertexBuffer>, Buffer::BufferErrors>;
        static BufferResult Create(const Buffer::CreateFromParams& params);

    public:
        ~VertexBuffer();

    private:
        VertexBuffer();
    };
}

/*
    Index Buffer
*/

namespace Graphics
{
    class IndexBuffer final : public Buffer
    {
    public:
        using BufferResult = Common::Result<std::unique_ptr<IndexBuffer>, Buffer::BufferErrors>;
        static BufferResult Create(const Buffer::CreateFromParams& params);

    public:
        ~IndexBuffer();

        GLenum GetElementType() const override;

    private:
        IndexBuffer();
    };
}

/*
    Instance Buffer
*/

namespace Graphics
{
    class InstanceBuffer final : public Buffer
    {
    public:
        using BufferResult = Common::Result<std::unique_ptr<InstanceBuffer>, Buffer::BufferErrors>;
        static BufferResult Create(const Buffer::CreateFromParams& params);

    public:
        ~InstanceBuffer();

        bool IsInstanced() const override
        {
            return true;
        }

    private:
        InstanceBuffer();
    };
}
