#include "model/Grid.h"
#include "Device.h"
#include "Log.h"
#include "Vertex.h"

Grid::Grid(const std::shared_ptr<Device> &device, int halfSize) : Model(device) {
  // Vertex buffer
  std::vector<Vertex> vertices{};
  for (int i = -halfSize; i <= halfSize; ++i) {
    // column
    vertices.push_back({.position = glm::vec3(i, 0, -halfSize), .color = glm::vec3(0, 0, 0)});
    vertices.push_back({.position = glm::vec3(i, 0, halfSize), .color = glm::vec3(0, 0, 0)});
    // row
    vertices.push_back({.position = glm::vec3(-halfSize, 0, i), .color = glm::vec3(0, 0, 0)});
    vertices.push_back({.position = glm::vec3(halfSize, 0, i), .color = glm::vec3(0, 0, 0)});
  }
  auto size = vertices.size() * sizeof(Vertex);

  auto buf      = std::make_shared<Buffer>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      size, (void *)vertices.data());
  mVertexBuffer = std::make_unique<VertexBuffer>(device, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                 vertices.size(), sizeof(Vertex));
  mVertexBuffer->copy(buf, device->getGraphicsQueue());

  // Index buffer
  std::vector<uint32_t> indices{};
  for (int i = 0; i < 2 * halfSize + 1; ++i) {
    indices.push_back(i * 4);
    indices.push_back(i * 4 + 1);
    indices.push_back(i * 4 + 2);
    indices.push_back(i * 4 + 3);
  }
  size = indices.size() * sizeof(uint32_t);

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

Grid::~Grid() { log_func; }
