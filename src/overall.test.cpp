#include "FrameworkCore/Camera.h"
#include "FrameworkCore/ContextManager.h"
#include "FrameworkCore/Framebuffer.h"
#include "FrameworkCore/MainWindow.h"
#include "FrameworkCore/Model.h"
#include "FrameworkCore/Shader.h"

#include "Utility/IO/IOExtension.h"
#include "Utility/IO/IniFile.h"

#include <imgui.h>

#include <chrono>
#include <iostream>
#include <numbers>

using namespace OpenGLFramework;

void SetMVP(float width, float height, float near, float far,
            Core::BasicTriRenderModel &model, Core::Camera &camera,
            Core::Shader &shader) {
  auto modelMat = model.transform.GetModelMatrix();
  shader.SetMat4("model", modelMat);

  auto viewMat = camera.GetViewMatrix();
  shader.SetMat4("view", viewMat);

  auto projectionMat =
      glm::perspective(glm::radians(camera.fov), width / height, near, far);
  shader.SetMat4("projection", projectionMat);

  return;
}

int main() {
  std::filesystem::path configPath =
      "../../../../../Resources/Configs/config2.ini";
  IOExtension::IniFile file{configPath};

  std::string pathSectionName = "path", nameSectionName = "name";

  auto nullableSection = file.rootSection.GetSubsection(pathSectionName);
  if (!nullableSection.has_value())
    IOExtension::LogError("Not found section " + pathSectionName);
  auto &pathSection = nullableSection->get();

  nullableSection = file.rootSection.GetSubsection(nameSectionName);
  if (!nullableSection.has_value())
    IOExtension::LogError("Not found section " + nameSectionName);
  auto &nameSection = nullableSection->get();
  std::string windowName = nameSection("window_name");

  auto t1 = std::chrono::steady_clock::now(), t2 = t1;
  [[maybe_unused]] auto &contextManager = Core::ContextManager::GetInstance();

  Core::MainWindow mainWindow{800, 600, windowName.c_str()};
  Core::BasicTriRenderModel model{pathSection("resource_dir")};
  Core::Shader shader{pathSection("vertex_shader_dir"),
                      pathSection("fragment_shader_dir")};
  Core::Camera frontCamera{{0, 10, 35}, {0, 1, 0}, {0, 0, -1}};
  t2 = std::chrono::steady_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(t2 -
                                                                         t1)
                   .count();

  Core::Camera sideCamera{{-30, 10, 18}, {0, 1, 0}, {30, 0, -18}};
  Core::Framebuffer frameBuffer;

  float near = 0.1f, far = 100.0f;
  // mainWindow.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  mainWindow.Register(
      [&shader, &model, &frontCamera, &mainWindow, near, far]() {
        glEnable(GL_DEPTH_TEST);
        // shader.Activate();
        // const auto [width, height] = mainWindow.GetWidthAndHeight();
        // SetMVP(static_cast<float>(width), static_cast<float>(height), near,
        // far,
        //        model, frontCamera, shader);
        // model.Draw(shader);
      });

  mainWindow.Register(
      [&shader, &model, &sideCamera, &frameBuffer, near, far]() {
        ImGui::Begin("Side");
        static bool init = true;
        if (init) {
          ImGui::SetWindowPos({50, 200});
          ImGui::SetWindowSize({200, 350});
          init = false;
        }

        [[maybe_unused]] const auto &io = ImGui::GetIO();

        auto subwindowSize = ImGui::GetWindowSize();

        if ((subwindowSize.x != frameBuffer.GetWidth() ||
             subwindowSize.y != frameBuffer.GetHeight()) &&
            !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
          glViewport(0, 0, static_cast<size_t>(subwindowSize.x),
                     static_cast<size_t>(subwindowSize.y));
          frameBuffer.Resize(static_cast<size_t>(subwindowSize.x),
                             static_cast<size_t>(subwindowSize.y));
          SetMVP(subwindowSize.x, subwindowSize.y, near, far, model, sideCamera,
                 shader);
          model.Draw(shader, frameBuffer);
        }

        ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(
                         frameBuffer.GetTextureColorBuffer())),
                     subwindowSize, {0, 1}, {1, 0});
        ImGui::End();
      });

  mainWindow.Register([&model]() {
    static bool init = true;
    ImGui::Begin("Model Adjustment");
    if (init) {
      ImGui::SetWindowPos({50, 50});
      ImGui::SetWindowSize({250, 100});
      init = false;
    }
    static glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    ImGui::SliderFloat3("rotation", &rotation.x, 0.0f,
                        2 * std::numbers::pi_v<float>);
    model.transform.rotation = glm::quat(rotation);

    static glm::vec3 position = {0.0f, 0.0f, 0.0f};
    ImGui::SliderFloat3("position", &position.x, -10, 10);
    model.transform.position = position;

    static float scale = 1.0f;
    ImGui::SliderFloat("scale", &scale, 0.5, 2);
    model.transform.scale = {scale, scale, scale};
    ImGui::End();
  });

  mainWindow.BindScrollCallback([&frontCamera](double, double yOffset) {
    float afterFov = frontCamera.fov - static_cast<float>(yOffset);
    frontCamera.fov = glm::clamp(afterFov, 15.0f, 75.0f);
  });

  // NOTE that code below(glfw) will reset the existing imgui handle, so
  // io.WantCaptureMouse will always be false. See
  // https://stackoverflow.com/a/72509936/15582103 for more info.
  //
  // mainWindow.BindCursorPosCallback([&frontCamera](double xPos, double yPos) {
  //	static float lastxPos = static_cast<float>(xPos),
  //				 lastyPos = static_cast<float>(yPos);
  //	float xOffset = lastxPos - static_cast<float>(xPos),
  //		  yOffset = lastyPos - static_cast<float>(yPos);
  //	lastxPos = static_cast<float>(xPos), lastyPos =
  // static_cast<float>(yPos); 	frontCamera.Rotate(
  // static_cast<float>(xOffset) * frontCamera.mouseSensitivity, { 0, 1, 0 });
  // frontCamera.Rotate( 		static_cast<float>(yOffset) *
  // frontCamera.mouseSensitivity, { 1, 0, 0 });
  //});

  mainWindow.BindKeyPressing<GLFW_KEY_W>([&frontCamera, &mainWindow]() {
    frontCamera.Translate(mainWindow.deltaTime *
                          glm::vec3{0.0f, 0.0f, -frontCamera.movementSpeed});
  });
  mainWindow.BindKeyPressing<GLFW_KEY_S>([&frontCamera, &mainWindow]() {
    frontCamera.Translate(mainWindow.deltaTime *
                          glm::vec3{0.0f, 0.0f, frontCamera.movementSpeed});
  });
  mainWindow.BindKeyPressing<GLFW_KEY_A>([&frontCamera, &mainWindow]() {
    frontCamera.Translate(mainWindow.deltaTime *
                          glm::vec3{-frontCamera.movementSpeed, 0.0f, 0.0f});
  });
  mainWindow.BindKeyPressing<GLFW_KEY_D>([&frontCamera, &mainWindow]() {
    frontCamera.Translate(mainWindow.deltaTime *
                          glm::vec3{frontCamera.movementSpeed, 0.0f, 0.5f});
  });
  mainWindow.BindKeyPressing<GLFW_KEY_UP>([&frontCamera, &mainWindow]() {
    frontCamera.Rotate(mainWindow.deltaTime * frontCamera.rotationSpeed,
                       {1, 0, 0});
  });
  mainWindow.BindKeyPressing<GLFW_KEY_DOWN>([&frontCamera, &mainWindow]() {
    frontCamera.Rotate(-mainWindow.deltaTime * frontCamera.rotationSpeed,
                       {1, 0, 0});
  });
  mainWindow.BindKeyPressing<GLFW_KEY_LEFT>(
      [&shader, &model, &frontCamera, &mainWindow]() {
        std::cout << "Pressed\n";
        shader.Activate();
        const auto [width, height] = mainWindow.GetWidthAndHeight();
        SetMVP(static_cast<float>(800), static_cast<float>(600), 0.1f, 100,
               model, frontCamera, shader);
        model.Draw(shader);
        // frontCamera.Rotate(mainWindow.deltaTime *
        // frontCamera.rotationSpeed,
        //                    {0, 1, 0});
      });
  mainWindow.BindKeyPressing<GLFW_KEY_RIGHT>([&frontCamera, &mainWindow]() {
    frontCamera.Rotate(-mainWindow.deltaTime * frontCamera.rotationSpeed,
                       {0, 1, 0});
  });

  mainWindow.BindKeyPressed<GLFW_KEY_SPACE>([&frontCamera]() {
    std::cout << "Pressed\n";
    frontCamera.Translate({0, 1.0f, 0.0f});
  });
  mainWindow.BindKeyReleased<GLFW_KEY_SPACE>([&frontCamera]() {
    std::cout << "Released\n";
    frontCamera.Translate({0, -1.0f, 0.0f});
  });
  mainWindow.BindKeyPressed<GLFW_KEY_ESCAPE>(
      [&mainWindow]() { mainWindow.Close(); });
  mainWindow.MainLoop({1.0f, 1.0f, 1.0f, 1.0f});
  return 0;
}
