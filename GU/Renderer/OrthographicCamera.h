/*
 * @Author: ImGili
 * @Description: 正交相机
 */
#pragma once
#include"Core/Core.h"
#include<glm/glm.hpp>
namespace GU
{
    class OrthographicCamera
    {
    public:
        OrthographicCamera(float left, float rigth, float bottom, float top);

        void RecalculateViewMatrix();
        void SetPosition(glm::vec3 position) { m_Position = position; RecalculateViewMatrix();}
        void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
        const glm::mat4& GetProjectionViewMatrix() const { return m_ProjectionViewMatrix;}
        void SetProjection(float left, float right, float bottom, float top);
    private:
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionViewMatrix;
        glm::vec3 m_Position;
        float m_Rotation = 0.0f;
    };
}