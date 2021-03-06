/*
 * @Author: ImGili
 * @Description: 
 */
#pragma once
#include"Core.h"
#include"Events/Event.h"
#include"Core/TimeStep.h"
#include<string>
namespace GU
{
    class Layer
    {
    public:
        Layer(const char* name);
        virtual ~Layer() = default;

        virtual void OnAttach(){}
        virtual void OnUpdate(TimeStep ts){}
        virtual void OnEvent(Event& e){}
        virtual void OnDetach(){}
        virtual void OnImGuiRender(){}
    private:
        std::string m_layer_name;
    };
}