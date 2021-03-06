/*
 * @Author: ImGili
 * @Description: 
 */
#include"Application.h"
#include"Renderer/RenderCommand.h"
#include"Renderer/Renderer.h"
#include"Core/TimeStep.h"
#include<glfw/glfw3.h>
#include<iostream>
using namespace GU;

Application* Application::s_Instance = nullptr;

Application::Application()
{
    #ifdef GU_PLATFORM_WINDOWS
        m_window = Window::Create({"GU", 1280, 720});
    #else
        m_window = Window::Create({"GU", 640, 400});
    #endif
    
    Renderer::Init();

    m_window->SetEventCallback([this](Event& event){this->OnEvent(event);});
    if (s_Instance != nullptr)
    {
        GU_DEBUGBREAK();
    }
    s_Instance = this;
    m_imGuiLayer = new ImGuiLayer();
    m_imGuiLayer->OnAttach();
}
Application::~Application()
{
    
}

void Application::Run()
{
    m_LastFrameTime = (float)glfwGetTime();
    while (m_Running)
    {
        RenderCommand::Clear();
        m_imGuiLayer->Begin();
        float time = (float)glfwGetTime();
        TimeStep timestep = time - m_LastFrameTime;
        m_LastFrameTime = time;
        for (auto it = m_Layers.begin(); it!=m_Layers.end(); it++)
        {
            (*it)->OnImGuiRender();
            (*it)->OnUpdate(timestep);
        }
        // for (auto it = m_Layers.begin(); it!=m_Layers.end(); it++)
        // {
        //     (*it)->OnImGuiRender();
        // }
        m_imGuiLayer->End();
        m_window->OnUpdate();
    }
    
}

void Application::OnEvent(Event& e)
{
    if(e.GetEventType() == EventType::WindowsCloseEvent)
        m_Running = false;
    for (auto it = m_Layers.begin(); it!=m_Layers.end(); it++)
    {
        if (e.Handled)
        {
            break;
        }
        
        (*it)->OnEvent(e);
    }
}

void Application::PushLayer(Layer* layer)
{
    m_Layers.push_back(layer);
    layer->OnAttach();
}

