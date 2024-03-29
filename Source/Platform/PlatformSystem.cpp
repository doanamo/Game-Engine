/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Platform/PlatformSystem.hpp"
using namespace Platform;

namespace
{
    int InstanceCounter = 0;

    void ErrorCallback(int error, const char* description)
    {
        LOG_ERROR("GLFW Error: {}", description);
    }
}

PlatformSystem::PlatformSystem() = default;
PlatformSystem::~PlatformSystem()
{
    if(m_attached)
    {
        ASSERT(InstanceCounter > 0, "Invalid instance count!");
        InstanceCounter--;

        if(InstanceCounter == 0)
        {
            // Terminate GLFW library along with last instance.
            glfwTerminate();
        }
    }
}

bool PlatformSystem::OnAttach(const Core::EngineSystemStorage& engineSystems)
{
    if(InstanceCounter == 0)
    {
        // Initialize GLFW library for first instance.
        glfwSetErrorCallback(ErrorCallback);

        if(!glfwInit())
        {
            LOG_ERROR("Could not initialize GLFW library!");
            return false;
        }

        InstanceCounter++;

        // Write GLFW details to log.
        int major, minor, revision;
        glfwGetVersion(&major, &minor, &revision);
        LOG_INFO("Using GLFW {}.{}.{} library.", major, minor, revision);
    }

    return m_attached = true;
}
