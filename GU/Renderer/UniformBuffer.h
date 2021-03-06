/*
 * @Author: ImGili
 * @Description: 
 */
#pragma once
#include"Core/Core.h"
#include<memory>
namespace GU
{
    class UniformBuffer
    {
    public:
        virtual ~UniformBuffer() = default;
        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
        virtual void Bind(uint32_t binding = 0) = 0;
        virtual void Unbind() = 0;
        static std::shared_ptr<UniformBuffer> Create(uint32_t size);
    };
}