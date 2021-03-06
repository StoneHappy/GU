/*
 * @Author: ImGili
 * @Description: 
 */
#pragma once
#include"Core/Core.h"
#include"glm/glm.hpp"
#include"Renderer/OrthographicCamera.h"
#include"Renderer/EditorCamera.h"
#include"Renderer/Camera.h"

#include"Scene/Component.h"
namespace GU
{
    // TODO: 2D Renderer
    class Renderer2D
    {
    public:
        static void Init();

        static void BeginScene(const OrthographicCamera& camera);
        static void BeginScene(const EditorCamera& camera);
        static void BeginScene(const Camera& camera, const glm::mat4& transform);

        static void DrawQuad(const glm::vec2& position, const glm::vec4& color);
        static void DrawQuad(const glm::vec2& position, const glm::vec2 size, const glm::vec4& color = {1.0f, 0.5f, 0.2f, 1.0f});
        static void DrawQuad(const glm::mat4& tranfom, const glm::vec4& color , int entityID);
        static void DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, const glm::vec4& color, int entityID);
        static void DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& sprite, int entityID);

        static void EndScene();

        static void Flush();

        // Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();
    };
}