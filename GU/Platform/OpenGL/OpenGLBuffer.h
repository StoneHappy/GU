/*
 * @Author: ImGili
 * @Description: 
 */
#pragma once
#include"Renderer/Buffer.h"
namespace GU
{
    class GU_API OpenGLVertexBuffer : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(float* data, uint32_t size);
        virtual ~OpenGLVertexBuffer();
        void Bind() override;
        void Unbind() override;

        void SetData(float* data, uint32_t size) override;
    private:
        uint32_t m_RendererID;
    };
}