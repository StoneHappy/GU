/*
 * @Author: ImGili
 * @Description: 
 */
#include"Core/Application.h"
#include"Core/Log.h"
#include"Core/Core.h"
#include<glad/glad.h>
GU::Application* GU::CreateApplication();
int main()
{
    GU::Log::Init();
    GU::Application* app = GU::CreateApplication();
    GU_ERROR("hello word!");
    app->Run();
    return 0;
}
