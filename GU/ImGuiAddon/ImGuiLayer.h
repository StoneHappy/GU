/*
 * @Author: ImGili
 * @Description: 
 */
#pragma once
#include"Core/Core.h"
#include"Core/Layer.h"
#include"Events/Event.h"

namespace GU
{
    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        void OnAttach() override;
        void OnDetach() override;
        void Begin();
        void End();

        void SetDarkThemeColors();
        void SetModernDarkTheme();
    };
}