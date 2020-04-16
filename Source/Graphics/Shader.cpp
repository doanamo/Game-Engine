/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#include "Graphics/Shader.hpp"
#include "Graphics/RenderContext.hpp"
#include <System/FileSystem.hpp>
using namespace Graphics;

namespace
{
    struct ShaderType
    {
        const char* name;
        const char* define;
        GLenum type;
    };

    const int ShaderTypeCount = 3;
    const ShaderType ShaderTypes[ShaderTypeCount] =
    {
        { "vertex shader",   "VERTEX_SHADER",   GL_VERTEX_SHADER   },
        { "geometry shader", "GEOMETRY_SHADER", GL_GEOMETRY_SHADER },
        { "fragment shader", "FRAGMENT_SHADER", GL_FRAGMENT_SHADER },
    };
}

Shader::Shader() = default;

Shader::~Shader()
{
    if(m_handle != OpenGL::InvalidHandle)
    {
        glDeleteProgram(m_handle);
        OpenGL::CheckErrors();
    }
}

Shader::InitializeResult Shader::Initialize(const LoadFromString& params)
{
    LOG("Creating shader...");
    LOG_SCOPED_INDENT();

    // Setup initialization guard.
    VERIFY(!m_initialized, "Instance has already been initialized!");
    SCOPE_GUARD_IF(!m_initialized, this->Reset());

    // Check arguments.
    CHECK_ARGUMENT_OR_RETURN(params.fileSystem != nullptr, Failure(InitializeErrors::InvalidArgument));
    CHECK_ARGUMENT_OR_RETURN(params.renderContext != nullptr, Failure(InitializeErrors::InvalidArgument));
    CHECK_ARGUMENT_OR_RETURN(!params.shaderCode.empty(), Failure(InitializeErrors::InvalidArgument));

    m_renderContext = params.renderContext;

    // Create array of shader objects for each type that can be linked.
    GLuint shaderObjects[ShaderTypeCount] = { 0 };

    SCOPE_GUARD_BEGIN();
    {
        for(int i = 0; i < ShaderTypeCount; ++i)
        {
            // Delete shaders after we link them into a program.
            glDeleteShader(shaderObjects[i]);
        }
    }
    SCOPE_GUARD_END();

    // Create mutable copy of the provided shader code.
    std::string shaderCode = params.shaderCode;

    // Extract shader version.
    std::string shaderVersion;

    std::size_t versionStart = shaderCode.find("#version ");
    std::size_t versionEnd = shaderCode.find('\n', versionStart);

    if(versionStart != std::string::npos)
    {
        shaderVersion = shaderCode.substr(versionStart, versionEnd - versionStart + 1);
        shaderCode.erase(versionStart, versionEnd + 1);
    }

    // Compile shader objects.
    bool shaderObjectsFound = false;

    for(unsigned int i = 0; i < ShaderTypeCount; ++i)
    {
        const ShaderType& shaderType = ShaderTypes[i];
        GLuint& shaderObject = shaderObjects[i];

        // Compile shader object if found.
        if(shaderCode.find(shaderType.define) != std::string::npos)
        {
            shaderObjectsFound = true;

            LOG_INFO("Compiling {}...", shaderType.name);

            // Create shader object.
            shaderObject = glCreateShader(shaderType.type);
            OpenGL::CheckErrors();

            if(shaderObject == OpenGL::InvalidHandle)
            {
                LOG_ERROR("Shader object could not be created!");
                return Failure(InitializeErrors::FailedShaderCreation);
            }

            // Prepare preprocessor define.
            std::string shaderDefine = "#define ";
            shaderDefine += shaderType.define;
            shaderDefine += "\n";

            // Compile shader object code.
            const char* shaderCodeSegments[] =
            {
                shaderVersion.c_str(),
                shaderDefine.c_str(),
                shaderCode.c_str(),
            };

            const std::size_t shaderCodeSegmentCount = Utility::StaticArraySize(shaderCodeSegments);

            glShaderSource(shaderObject,
                Utility::NumericalCast<GLsizei>(shaderCodeSegmentCount),
                (const GLchar**)&shaderCodeSegments, nullptr);
            OpenGL::CheckErrors();

            glCompileShader(shaderObject);
            OpenGL::CheckErrors();

            // Check compiling results.
            GLint compileStatus = 0;
            glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &compileStatus);
            OpenGL::CheckErrors();

            if(compileStatus == GL_FALSE)
            {
                LOG_ERROR("Shader object could not be compiled!");

                GLint errorLength = 0;
                glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &errorLength);
                OpenGL::CheckErrors();

                if(errorLength != 0)
                {
                    std::vector<char> errorText(errorLength);
                    glGetShaderInfoLog(shaderObject, errorLength, &errorLength, &errorText[0]);
                    OpenGL::CheckErrors();

                    LOG_ERROR("Shader compile errors: \"{}\"", errorText.data());
                }

                return Failure(InitializeErrors::FailedShaderCompilation);
            }
        }
    }

    // Check if any shader objects were found.
    if(shaderObjectsFound == false)
    {
        LOG_ERROR("Could not find any shader objects!");
        return Failure(InitializeErrors::FailedShaderCompilation);
    }

    // Create shader program.
    m_handle = glCreateProgram();
    OpenGL::CheckErrors();

    if(m_handle == OpenGL::InvalidHandle)
    {
        LOG_ERROR("Shader program could not be created!");
        return Failure(InitializeErrors::FailedProgramCreation);
    }

    // Attach compiled shader objects.
    for(unsigned int i = 0; i < ShaderTypeCount; ++i)
    {
        GLuint& shaderObject = shaderObjects[i];

        if(shaderObject != OpenGL::InvalidHandle)
        {
            glAttachShader(m_handle, shaderObject);
            OpenGL::CheckErrors();
        }
    }

    // Link attached shader objects.
    LOG_INFO("Linking shader program...");

    glLinkProgram(m_handle);
    OpenGL::CheckErrors();

    // Detach already linked shader objects.
    for(unsigned int i = 0; i < ShaderTypeCount; ++i)
    {
        GLuint& shaderObject = shaderObjects[i];

        if(shaderObject != OpenGL::InvalidHandle)
        {
            glDetachShader(m_handle, shaderObject);
            OpenGL::CheckErrors();
        }
    }

    // Check linking results.
    GLint linkStatus = 0;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &linkStatus);
    OpenGL::CheckErrors();

    if(linkStatus == GL_FALSE)
    {
        LOG_ERROR("Shader program could not be linked!");

        GLint errorLength = 0;
        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &errorLength);
        OpenGL::CheckErrors();

        if(errorLength != 0)
        {
            std::vector<char> errorText(errorLength);
            glGetProgramInfoLog(m_handle, errorLength, &errorLength, &errorText[0]);
            OpenGL::CheckErrors();

            LOG_ERROR("Shader link errors: \"{}\"", errorText.data());
        }

        return Failure(InitializeErrors::FailedProgramLinkage);
    }

    // Success!
    m_initialized = true;
    return Success();
}

Shader::InitializeResult Shader::Initialize(const LoadFromFile& params)
{
    LOG("Loading shader from \"{}\" file...", params.filePath);
    LOG_SCOPED_INDENT();

    // Setup initialization guard.
    SCOPE_GUARD_IF(!m_initialized, this->Reset());

    // Validate arguments.
    CHECK_ARGUMENT_OR_RETURN(params.fileSystem != nullptr, Failure(InitializeErrors::InvalidArgument));

    // Resolve file path.
    auto resolvePathResult = params.fileSystem->ResolvePath(params.filePath);

    if(!resolvePathResult)
    {
        LOG_ERROR("Could not resolve file path!");
        return Failure(InitializeErrors::FailedFilePathResolve);
    }

    // Load shader code from a file.
    std::string shaderCode = Utility::GetTextFileContent(resolvePathResult.Unwrap());

    if(shaderCode.empty())
    {
        LOG_ERROR("File could not be read!");
        return Failure(InitializeErrors::InvalidFileContent);
    }

    // Call base initialization method.
    LoadFromString compileParams;
    compileParams.fileSystem = params.fileSystem;
    compileParams.shaderCode = std::move(shaderCode);
    compileParams.renderContext = params.renderContext;

    return this->Initialize(compileParams);
}

GLint Shader::GetAttributeIndex(std::string name) const
{
    ASSERT(m_initialized, "Shader has not been initialized!");
    ASSERT(!name.empty(), "Attribute name cannot be empty!");

    GLint location = glGetAttribLocation(m_handle, name.c_str());
    return location;
}

GLint Shader::GetUniformIndex(std::string name) const
{
    ASSERT(m_initialized, "Shader has not been initialized!");
    ASSERT(!name.empty(), "Uniform name cannot be empty!");

    GLint location = glGetUniformLocation(m_handle, name.c_str());
    return location;
}

GLuint Shader::GetHandle() const
{
    ASSERT(m_initialized, "Shader has not been initialized!");
    return m_handle;
}

bool Shader::IsInitialized() const
{
    return m_initialized;
}
