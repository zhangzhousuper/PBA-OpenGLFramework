#include "ContextManager.h"
#include "MainWindow.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "Framebuffer.h"

#include <glm/glm.hpp>
#include <imgui.h>

#include <iostream>

using namespace OpenGLFramework::Core;

void SetMVP(float width, float height, float near, float far,
    BasicTriRenderModel& model, Camera& camera, Shader& shader)
{
    auto modelMat = model.transform.GetModelMatrix();
    shader.SetMat4("model", modelMat);

    auto viewMat = camera.GetViewMatrix();
    shader.SetMat4("view", viewMat);

    auto projectionMat = glm::perspective(glm::radians(camera.fov),
        width / height, near, far);
    shader.SetMat4("projection", projectionMat);

    return;
}

int main()
{
    [[maybe_unused]] ContextManager& manager = ContextManager::GetInstance();
    MainWindow window{ 800, 600, "Test" };

    BasicTriRenderModel model{ R"(D:\111\University\Course\CS\Computer Graphics\OpenGL)"
        R"(\OpenGLFramework\Resources\Models\Cube\CubeWithoutNormal.obj)" };
    Shader shader{ R"(D:\111\University\Course\CS\Computer Graphics\OpenGL)"
        R"(\OpenGLFramework\Shaders\Basic.vert)",
        R"(D:\111\University\Course\CS\Computer Graphics\OpenGL)"
        R"(\OpenGLFramework\Shaders\Basic.frag)" };
    Camera camera{ { 0.5, 0.7, 5}, {0, 0.8, 0.2}, {0, 0, -1} };
    camera.RotateAroundCenter(60, glm::vec3{ 0, 1, 0 }, { 0, 0, 0 });

    auto pos = camera.GetPosition();
    std::cout << pos.x << " " << pos.y << " " << pos.z << "\n";

    Framebuffer frameBuffer(400, 300);

    window.Register(
        [&shader, &model, &camera, &frameBuffer]() {
            glEnable(GL_DEPTH_TEST);
            shader.Activate();

            ImGui::Begin("Side");
            static bool init = true;
            if (init)
            {
                ImGui::SetWindowPos({ 50, 200 });
                ImGui::SetWindowSize({ 400, 300 });
                init = false;
            }

            [[maybe_unused]] const auto& io = ImGui::GetIO();

            auto subwindowSize = ImGui::GetWindowSize();

            if ((subwindowSize.x != frameBuffer.GetWidth() ||
                subwindowSize.y != frameBuffer.GetHeight()) &&
                !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                glViewport(0, 0, static_cast<size_t>(subwindowSize.x),
                    static_cast<size_t>(subwindowSize.y));
                frameBuffer.Resize(static_cast<size_t>(subwindowSize.x),
                    static_cast<size_t>(subwindowSize.y));
                SetMVP(subwindowSize.x, subwindowSize.y, 4, 10, model,
                    camera, shader);
                model.Draw(shader, frameBuffer);
            }

            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(
                frameBuffer.GetTextureColorBuffer())), subwindowSize, { 0, 1 }, { 1, 0 });
            ImGui::End();
        });

    window.MainLoop(glm::vec4{ 0, 0, 0, 1 });
    return 0;
}