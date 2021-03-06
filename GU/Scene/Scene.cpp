/*
 * @Author: ImGili
 * @Description:
 */
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Component.h"
#include "Renderer/Renderer2D.h"
using namespace GU;
Scene::Scene()
{
}
Scene::~Scene()
{
}

Entity Scene::CreateEntity(const std::string &name)
{
    Entity entity = {m_Registry.create(), this};
    entity.AddComponent<TransformComponent>();
    auto &Tag = entity.AddComponent<TagComponent>();
    Tag.Tag = name.empty() ? "Entity" : name;
    return entity;
}

void Scene::OnUpdateRuntime(TimeStep ts)
{
    // Update Script Entity
    m_Registry.view<NativeScriptComponent>().each([&](entt::entity entity, NativeScriptComponent &nsc)
                                                  {
                                                      if (!nsc.Instance)
                                                      {
                                                          nsc.Instance = nsc.InstantiateScript();
                                                          nsc.Instance->m_Entity = {entity, this};
                                                          nsc.Instance->OnCreate();
                                                      }
                                                      nsc.Instance->OnUpdate(ts);
                                                  });

    Camera *mainCamera = nullptr;
    glm::mat4 cameraTransform;
    {
        auto view = m_Registry.view<TransformComponent, CameraComponent>();
        for (auto entity : view)
        {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

            if (camera.Primary)
            {
                mainCamera = &camera.Camera;
                cameraTransform = transform.GetTransform();
                break;
            }
        }
    }

    if (mainCamera)
    {
        Renderer2D::BeginScene(*mainCamera, cameraTransform);

        auto group = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
        }

        Renderer2D::EndScene();
    }
}

void Scene::OnUpdateEditor(TimeStep ts, EditorCamera &camera)
{
    Renderer2D::BeginScene(camera);

    auto group = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
    for (auto entity : group)
    {
        auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
        Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
    }

    Renderer2D::EndScene();
}

void Scene::OnViewportResize(uint32_t width, uint32_t height)
{
    m_ViewportWidth = width;
    m_ViewportHeight = height;

    // Resize our non-FixedAspectRatio cameras
    auto view = m_Registry.view<CameraComponent>();
    for (auto entity : view)
    {
        auto &cameraComponent = view.get<CameraComponent>(entity);
        cameraComponent.Camera.SetViewportSize(width, height);
    }
}

void Scene::DeleteEntity(Entity entity)
{
    m_Registry.destroy(entity);
}

template <typename T>
void Scene::OnComponentAdd(Entity entity, T component)
{
}

template <>
void Scene::OnComponentAdd(Entity entity, CameraComponent component)
{
    if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
    {
        component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }
}

Entity Scene::GetPrimaryCameraEntity()
{
    auto view = m_Registry.view<CameraComponent>();
    for (auto entity : view)
    {
        const auto &camera = view.get<CameraComponent>(entity);
        if (camera.Primary)
            return Entity{entity, this};
    }
    return {};
}