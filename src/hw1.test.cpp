#include "FrameworkCore/Camera.h"
#include "FrameworkCore/ContextManager.h"
#include "FrameworkCore/Framebuffer.h"
#include "FrameworkCore/MainWindow.h"
#include "FrameworkCore/Model.h"
#include "FrameworkCore/Shader.h"

#include "Utility/IO/IOExtension.h"
#include "Utility/IO/IniFile.h"
#include "glm/fwd.hpp"

// #include <imgui.h>

#include <chrono>
#include <iostream>
#include <numbers>

using namespace OpenGLFramework;

double xPos, yPos, lastxPos, lastyPos;

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
  std::filesystem::path configPath = "../../../../../Resources/Configs/HW1.ini";
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
  Core::BasicTriRenderModel model1{pathSection("resource_dir1")};
  Core::Shader shader1{pathSection("vertex_shader_dir1"),
                       pathSection("fragment_shader_dir1")};
  Core::BasicTriRenderModel model2{pathSection("resource_dir2")};
  Core::Shader shader2{pathSection("vertex_shader_dir2"),
                       pathSection("fragment_shader_dir2")};
  Core::Camera frontCamera{{0, 10, 35}, {0, 1, 0}, {0, 0, -1}};
  t2 = std::chrono::steady_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(t2 -
                                                                         t1)
                   .count()
            << std::endl;

  Core::Framebuffer frameBuffer;

  // mainWindow.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  float near = 0.1f, far = 100.0f;
  mainWindow.Register([&shader2, &model2, &shader1, &model1, &model, &shader,
                       &frontCamera, &mainWindow, near, far]() {
    glEnable(GL_DEPTH_TEST);

    const auto [width, height] = mainWindow.GetWidthAndHeight();

    shader1.Activate();
    SetMVP(static_cast<float>(width), static_cast<float>(height), near, far,
           model1, frontCamera, shader1);
    model1.Draw(shader1);

    // shader.Activate();
    // SetMVP(static_cast<float>(width), static_cast<float>(height), near, far,
    //        model, frontCamera, shader);
    // model.Draw(shader);

    // shader2.Activate();
    // SetMVP(static_cast<float>(width), static_cast<float>(height), near, far,
    //        model2, frontCamera, shader2);
    // model2.Draw(shader2);
  });

  // mainWindow.Register(
  //     [&shader, &model, &frontCamera, &mainWindow, near, far]() {
  //       glEnable(GL_DEPTH_TEST);
  //       shader.Activate();
  //       const auto [width, height] = mainWindow.GetWidthAndHeight();
  //       SetMVP(static_cast<float>(width), static_cast<float>(height), near,
  //       far,
  //              model, frontCamera, shader);
  //       model.Draw(shader);
  //     });

  // mainWindow.Register([&model]() {
  //   static bool init = true;
  //   ImGui::Begin("Model Adjustment");
  //   if (init) {
  //     ImGui::SetWindowPos({50, 50});
  //     ImGui::SetWindowSize({250, 100});
  //     init = false;
  //   }

  //   static glm::vec3 position = {0.0f, 0.0f, 0.0f};
  //   ImGui::SliderFloat2("position", &position.x, -10, 10);
  //   model.transform.position = position;

  //   static float scale = 1.0f;
  //   ImGui::SliderFloat("scale", &scale, 0.5, 2);
  //   model.transform.scale = {scale, scale, scale};
  //   ImGui::End();
  // });

  mainWindow.BindScrollCallback([&frontCamera](double, double yOffset) {
    float afterFov = frontCamera.fov - static_cast<float>(yOffset);
    frontCamera.fov = glm::clamp(afterFov, 15.0f, 75.0f);
  });

  // NOTE that code below(glfw) will reset the existing imgui handle, so
  // io.WantCaptureMouse will always be false. See
  // https://stackoverflow.com/a/72509936/15582103 for more info.
  //
  // mainWindow.BindCursorPosCallback([&frontCamera](double xPos, double yPos) {
  //   static float lastxPos = static_cast<float>(xPos),
  //                lastyPos = static_cast<float>(yPos);
  //   float xOffset = lastxPos - static_cast<float>(xPos),
  //         yOffset = lastyPos - static_cast<float>(yPos);
  //   lastxPos = static_cast<float>(xPos), lastyPos = static_cast<float>(yPos);
  //   frontCamera.Rotate(
  //       static_cast<float>(xOffset) * frontCamera.mouseSensitivity, {0, 1,
  //       0});
  //   frontCamera.Rotate(
  //       static_cast<float>(yOffset) * frontCamera.mouseSensitivity, {1, 0,
  //       0});
  // });

  // mainWindow.BindMouseButtonCallback([&frontCamera](double xPos, double yPos)
  // {
  //   static float lastxPos = static_cast<float>(xPos),
  //                lastyPos = static_cast<float>(yPos);
  //   float xOffset = lastxPos - static_cast<float>(xPos),
  //         yOffset = lastyPos - static_cast<float>(yPos);
  //   glm::vec2 mousePos = {static_cast<float>(xOffset),
  //                         static_cast<float>(yOffset)};

  //   std::cout << "mousePos: " << mousePos.x << ", " << mousePos.y <<
  //   std::endl; std::cout << "lastxPos: " << lastxPos << ", " << lastyPos <<
  //   std::endl; std::cout << "xPos: " << xPos << ", " << yPos << std::endl;
  // });

  mainWindow.BindMouseButtonCallback([](int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      glfwGetCursorPos(glfwGetCurrentContext(), &xPos, &yPos);
      lastxPos = xPos, lastyPos = yPos;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      glfwGetCursorPos(glfwGetCurrentContext(), &xPos, &yPos);

      std::cout << "xPos: " << xPos << ", yPos: " << yPos << std::endl;
      std::cout << "lastxPos: " << lastxPos << ", lastyPos: " << lastyPos
                << std::endl;
      float xOffset = lastxPos - static_cast<float>(xPos),
            yOffset = lastyPos - static_cast<float>(yPos);
      glm::vec2 mousePos = {static_cast<float>(xOffset),
                            static_cast<float>(yOffset)};

      std::cout << "mousePos: " << mousePos.x << ", " << mousePos.y
                << std::endl;
    }
  });

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
        float near = 0.1f, far = 100.0f;
        shader.Activate();
        const auto [width, height] = mainWindow.GetWidthAndHeight();
        SetMVP(static_cast<float>(width), static_cast<float>(height), near, far,
               model, frontCamera, shader);
        model.Draw(shader);
        // frontCamera.Rotate(mainWindow.deltaTime * frontCamera.rotationSpeed,
        //                    {0, 1, 0});
      });

  mainWindow.BindKeyPressing<GLFW_KEY_RIGHT>(
      [&shader, &model2, &frontCamera, &mainWindow]() {
        std::cout << "Pressed\n";
        float near = 0.1f, far = 100.0f;
        shader.Activate();
        const auto [width, height] = mainWindow.GetWidthAndHeight();
        SetMVP(static_cast<float>(width), static_cast<float>(height), near, far,
               model2, frontCamera, shader);
        model2.Draw(shader);
        // frontCamera.Rotate(mainWindow.deltaTime * frontCamera.rotationSpeed,
        //                    {0, 1, 0});
      });

  mainWindow.BindKeyPressed<GLFW_KEY_SPACE>([&frontCamera]() {
    std::cout << "Pressed\n";
    frontCamera.Translate({0, 1.0f, 0.0f});
  });
  mainWindow.BindKeyReleased<GLFW_KEY_SPACE>(
      [&shader, &model, &frontCamera]() { std::cout << "Released\n"; });

  // void ApplyForceFromMouse(double xPos, double yPos);
  // mainWindow.BindCursorPosCallback(ApplyForceFromMouse);

  mainWindow.BindKeyPressed<GLFW_KEY_ESCAPE>(
      [&mainWindow]() { mainWindow.Close(); });
  mainWindow.MainLoop({1.0f, 1.0f, 1.0f, 1.0f});
  return 0;
}
