#pragma once

#include <memory>

#include "IndexBuffer.h"
#include "VertexBuffer.h"

class Device;

class Model {
public:
  explicit Model(const std::shared_ptr<Device> &device);
  ~Model();

  [[nodiscard]] const std::unique_ptr<VertexBuffer> &getVertexBuffer() const {
    return mVertexBuffer;
  }
  [[nodiscard]] const std::unique_ptr<IndexBuffer> &getIndexBuffer() const { return mIndexBuffer; }

protected:
  const std::shared_ptr<Device> mDevice       = nullptr;
  std::unique_ptr<VertexBuffer> mVertexBuffer = nullptr;
  std::unique_ptr<IndexBuffer>  mIndexBuffer  = nullptr;
};
