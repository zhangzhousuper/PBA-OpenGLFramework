#include "MainWindow.h"
#include "ContextManager.h"


#include <iostream>

using namespace OpenGLFramework::Core;

int main() {
  [[maybe_unused]] ContextManager &manager = ContextManager::GetInstance();
  MainWindow mainWindow{800, 600, "Title test"};
  mainWindow.Register([]() {
    static int i = 0;
    if (i < 10)
      std::cout << i;
    i++;
  });

  mainWindow.BindKeyPressing<GLFW_KEY_W>(
      [&mainWindow]() { mainWindow.Close(); });

  mainWindow.MainLoop({0, 0, 0, 1});
  return 0;
}