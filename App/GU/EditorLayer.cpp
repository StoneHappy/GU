/*
 * @Author: ImGili
 * @Description:
 */
#include "EditorLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/Base.h"
#include "Renderer/FrameBuffer.h"
#include "Renderer/RenderCommand.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "ImGuiAddon/FileBrowser/ImGuiFileBrowser.h"
#include <ImGuizmo.h>
#include "Scene/Component.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/ScriptableEntity.h"
#include "Scene/SceneSerializer.h"
#include <imgui.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "GLFW/glfw3.h"
namespace GU
{
    extern const std::filesystem::path g_AssetPath;
}

EditorLayer::EditorLayer()
    : Layer("EditorLayer")
{
}

void EditorLayer::OnUpdate(TimeStep ts)
{
    if (m_ViewportSize.x != 0 && m_ViewportSize.y != 0 && m_FrameBuffer->GetSpec().Width != m_ViewportSize.x || m_FrameBuffer->GetSpec().Height != m_ViewportSize.y)
    {
        m_FrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
    }
    RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 1.0f});
    Renderer2D::ResetStats();
    // m_EditorCamera.OnUpdate(ts);
    m_FrameBuffer->Bind();
    RenderCommand::Clear();
    // m_ActiveScene->OnUpdate(ts);
    // m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

    switch (m_SceneState)
    {
        case SceneState::Edit:
        {
            m_EditorCamera.OnUpdate(ts);

            m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
            break;
        }
        case SceneState::Play:
        {
            m_ActiveScene->OnUpdateRuntime(ts);
            break;
        }
    }

    // 获取鼠标位置，并转换到贴图坐标
    auto [mx, my] = ImGui::GetMousePos();
    mx -= m_ViewportBounds[0].x;
    my -= m_ViewportBounds[0].y;
    glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
    my = viewportSize.y - my;
    int mouseX = (int)mx;
    int mouseY = (int)my;

    if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
    {
        int pixelData = m_FrameBuffer->ReadPixel(1, mouseX, mouseY);
        m_HoveredEntity = pixelData == 0 ? Entity() : Entity((entt::entity)(pixelData - 1), m_ActiveScene.get());
        // GU_WARN("Pixel:{0}", pixelData);
    }

    m_FrameBuffer->Unbind();
}
void EditorLayer::OnImGuiRender()
{

    static bool p_open = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &p_open, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    float minWinStyle = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    style.WindowMinSize.x = minWinStyle;
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New...", "Ctrl+N"))
            {
                m_ActiveScene = std::make_shared<Scene>();
                m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O"))
            {
                m_OpenScene = true;
                ImGui::OpenPopup("Open File");
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
            {
                m_SaveScene = true;
            }
            if (ImGui::MenuItem("Exit"))
                Application::Get()->Close();
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    // Remember the name to ImGui::OpenPopup() and showFileDialog() must be same...
    if (m_OpenScene)
        ImGui::OpenPopup("Open File");
    if (m_SaveScene)
        ImGui::OpenPopup("Save File");

    /* Optional third parameter. Support opening only compressed rar/zip files.
     * Opening any other file will show error, return false and won't close the dialog.
     */
    static imgui_addons::ImGuiFileBrowser file_dialog;

    if (file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".gu"))
    {
        OpenScene(file_dialog.selected_path.c_str());
    }

    if (file_dialog.showFileDialog("Save File", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".gu"))
    {
        SceneSerializer sceneserializer(m_ActiveScene);
        sceneserializer.Serializer(file_dialog.selected_path.c_str());
    }

    m_SceneHierarchyPanel.OnImGuiRender();
    m_ContentBrowserPanel.OnImGuiRender();
    // {
    //     ImGui::ShowDemoWindow();
    // }
    m_OpenScene = false;
    m_SaveScene = false;

    ImGui::Begin("Stats");
    std::string HoveredEntityName = "None";
    if (m_HoveredEntity)
    {
        HoveredEntityName = m_HoveredEntity.GetComponent<TagComponent>().Tag;
    }

    ImGui::Text("Hovered Entity: %s", HoveredEntityName.c_str());
    auto stats = Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

    ImGui::End();

    //=============viewport======================================
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");
    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    // viewport最大包围边
    m_ViewportBounds[0] = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
    m_ViewportBounds[1] = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};

    m_IsViewportFocus = ImGui::IsWindowFocused();
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
    uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();

    ImGui::Image(reinterpret_cast<void *>(textureID), ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1, 0});
    Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
        {
            const wchar_t *path = (const wchar_t *)payload->Data;
            OpenScene(std::filesystem::path(g_AssetPath) / path);
        }
        ImGui::EndDragDropTarget();
    }

    if (selectedEntity && m_GizmoType != -1)
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
        // auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
        const glm::mat4 &cameraProjection = m_EditorCamera.GetProjection();
        glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

        auto &tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                             (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

        if (ImGuizmo::IsUsing())
        {
            glm::vec3 scale;
            glm::vec3 rotation;
            glm::vec3 translation;
            Math::Decompose(transform, scale, rotation, translation);
            tc.Translation = translation;
            glm::vec3 deltaRotation = rotation - tc.Rotation;
            tc.Rotation += deltaRotation;
            tc.Scale = scale;
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    UI_ToolBar();
    ImGui::End();
}

void EditorLayer::UI_ToolBar()
{
    ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    float size = 50;
    std::shared_ptr<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
    ImGui::SameLine((ImGui::GetContentRegionMax().x*0.5) - (size*0.5));
    if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
    {
        if (m_SceneState == SceneState::Edit)
            OnScenePlay();
        else if (m_SceneState == SceneState::Play)
            OnSceneStop();
    }
    ImGui::End();
}

void EditorLayer::OnAttach()
{
    // Application::Get()->GetWindow().MaxWindow();
    m_ActiveScene = std::make_shared<Scene>();
    m_IconPlay = Texture2D::Create("./resources/icons/PlayButton.png");
    m_IconStop = Texture2D::Create("./resources/icons/StopButton.png");
#if 0
    m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
    auto& cc = m_CameraEntity.AddComponent<CameraComponent>();

    m_SecondCameraEntity = m_ActiveScene->CreateEntity("Camera B");
    m_SecondCameraEntity.AddComponent<CameraComponent>();
    auto& scc = m_SecondCameraEntity.GetComponent<CameraComponent>();
    scc.Primary = false;
    class CameraController : public ScriptableEntity
    {
    public:
        virtual ~CameraController() = default;

        virtual void OnCreate() {}
        virtual void OnUpdate(TimeStep ts) {
            auto& translation = GetComponent<TransformComponent>().Translation;
            float speed = 5.0f;
            if(Input::IsKeyPressed(Key::A))
                translation.x -= speed * ts;
            if(Input::IsKeyPressed(Key::D))
                translation.x += speed * ts;
            if(Input::IsKeyPressed(Key::S))
                translation.y -= speed * ts;
            if(Input::IsKeyPressed(Key::W))
                translation.y += speed * ts;
        }
        virtual void OnDestroy() {}
    };

    m_SecondCameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();

    Entity entity1 = m_ActiveScene->CreateEntity();
    entity1.AddComponent<SpriteRendererComponent>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    auto& transform1 = entity1.GetComponent<TransformComponent>().Translation;
    transform1 = { 0.0, 0.0, 0.0 };
    Entity entity2 = m_ActiveScene->CreateEntity();
    entity2.AddComponent<SpriteRendererComponent>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    auto& sprite = entity2.GetComponent<SpriteRendererComponent>();
    sprite.Texture = Texture2D::Create("assets/textures/container2.png");
    auto& transform2 = entity2.GetComponent<TransformComponent>().Translation;
    transform1 = { 1.0, 1.0, 0.0 };
    Entity entity3 = m_ActiveScene->CreateEntity();
    entity3.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 0.0f, 1.0f, 1.0f});
    auto& transform3 = entity3.GetComponent<TransformComponent>().Translation;
    transform3 = { -1.0, -1.0, 0.0 };
#endif
    m_SceneHierarchyPanel.SetContext(m_ActiveScene);

    FrameBufferSpecification spec;
    spec.AttachmentsSpecification = {FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::Depth};
    spec.Height = 1280;
    spec.Width = 720;
    m_FrameBuffer = FrameBuffer::Create(spec);
    m_EditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
}

void EditorLayer::OnEvent(Event &e)
{
    EventProcesser eventProcesser(e);
    eventProcesser.Process<KeyPressedEvent>(GU_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    m_EditorCamera.OnEvent(e);
    eventProcesser.Process<MouseButtonPressedEvent>(GU_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent &e)
{
    bool control = Input::IsKeyPressed(Key::LeftCtrl);
    bool shift = Input::IsKeyPressed(Key::LeftShift);
    switch (e.GetKeyCode())
    {
    case Key::S:
    {
        if (control && shift)
        {
            m_SaveScene = true;
        }
        m_GizmoType = ImGuizmo::OPERATION::SCALE;
        break;
    }
    case Key::O:
    {
        if (control)
        {
            m_OpenScene = true;
        }
        break;
    }
    case Key::N:
    {
        if (control)
        {
            m_ActiveScene = std::make_shared<Scene>();
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        }
        break;
    }
    case Key::R:
    {
        m_GizmoType = ImGuizmo::OPERATION::ROTATE;
        break;
    }
    case Key::G:
    {
        m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
        break;
    }
    case Key::W:
    {
        m_GizmoType = -1;
        break;
    }

    default:
        break;
    }
    return false;
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
{
    if (e.GetMouseButton() == MouseKey::ButtonLeft)
    {
        if (m_HoveredEntity && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftShift))
        {
            m_SceneHierarchyPanel.SetSelectedEnttiy(m_HoveredEntity);
        }
    }
    return false;
}

void EditorLayer::OpenScene(const std::filesystem::path &path)
{
    if (path.extension().string() != ".gu")
    {
        GU_WARN("Could not load scene{0}, it is not a scene file", path.filename().string());
        return;
    }
    m_ActiveScene = std::make_shared<Scene>();
    m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    SceneSerializer sceneserializer(m_ActiveScene);
    sceneserializer.Deserializer(path.string().c_str());
}

void EditorLayer::OnScenePlay()
{
    m_SceneState = SceneState::Play;
}
void EditorLayer::OnSceneStop()
{
    m_SceneState = SceneState::Edit;
}