#include "model/Triangle.h"
#include "Device.h"
#include "Log.h"
#include "Vertex.h"

Triangle::Triangle(const std::shared_ptr<Device> &device) : Model(device) {
  // Vertex buffer
  const std::vector<Vertex> vertices = {
      {{4.0f, 0.0f, 4.0f}, {1.0f, 0.0f, 0.0f}},
      {{4.0f, 0.0f, -4.0f}, {0.0f, 1.0f, 0.0f}},
      {{-4.0f, 0.0f, -4.0f}, {0.0f, 0.0f, 1.0f}},
      {{-4.0f, 0.0f, 4.0f}, {0.0f, 1.0f, 1.0f}},
  };
  auto size = vertices.size() * sizeof(Vertex);

  auto buf      = std::make_shared<Buffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      size, (void *)vertices.data());
  mVertexBuffer = std::make_unique<VertexBuffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                 vertices.size(), sizeof(Vertex));
  mVertexBuffer->copy(buf, mDevice->getGraphicsQueue());

  // Index buffer
  const std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
  size                                = indices.size() * sizeof(uint32_t);

  buf.reset();
  buf = std::make_shared<Buffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 size, (void *)indices.data());

  mIndexBuffer =
      std::make_unique<IndexBuffer>(mDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indices.size(), size);
  mIndexBuffer->copy(buf, mDevice->getGraphicsQueue());
}

Triangle::~Triangle() { log_func; }
