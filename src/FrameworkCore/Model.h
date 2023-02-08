#pragma once
#ifndef OPENGLFRAMEWORK_CORE_MODEL_H_
#define OPENGLFRAMEWORK_CORE_MODEL_H_

#include "Framebuffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Transform.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <stack>
#include <vector>

namespace OpenGLFramework::Core {

template <typename T, typename... Args>
void LoadResourcesDecorator_(const aiScene *model, T processFunc,
                             Args &&...args) {
  aiNode *node = nullptr;
  std::stack<aiNode *> childNodes;
  childNodes.push(model->mRootNode);
  while (!childNodes.empty()) {
    node = childNodes.top();
    for (unsigned int i = 0u, len = node->mNumMeshes; i < len; i++) {
      aiMesh *mesh = model->mMeshes[node->mMeshes[i]];
      processFunc(mesh, std::forward<Args>(args)...);
    }
    childNodes.pop();
    for (unsigned int i = 0u, len = node->mNumChildren; i < len; i++) {
      childNodes.push(node->mChildren[i]);
    }
  }
};

class BasicTriModel {
public:
  BasicTriModel(const std::filesystem::path &modelPath);
  std::vector<BasicTriMesh> meshes;
  Transform transform;

private:
  void LoadResources_(const aiScene *model);
};

class BasicTriRenderModel {
public:
  TexturePool texturePool;
  std::vector<BasicRenderTriMesh> meshes;
  Transform transform;
  BasicTriRenderModel(const std::filesystem::path &modelPath,
                      bool textureNeedFlip = false);
  void Draw(Shader &shader);
  void Draw(Shader &shader, Framebuffer &buffer);

private:
  void LoadResources_(const aiScene *model,
                      const std::filesystem::path &resourceRootPath,
                      bool textureNeedFlip);
};

} // namespace OpenGLFramework::Core

#endif // !OPENGLFRAMEWORK_CORE_MODEL_H_
