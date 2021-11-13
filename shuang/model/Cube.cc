#include "model/Cube.h"
#include "Device.h"
#include "Logger.h"
#include "Vertex.h"

Cube::Cube(const std::shared_ptr<Device> &device) : Model(device) {
  // Vertex buffer
  const std::vector<Vertex> vertices = {
      {{-1.0f, -1.0f, -1.0f}, {0.583f, 0.771f, 0.014f}},
      {{-1.0f, -1.0f, 1.0f}, {0.609f, 0.115f, 0.436f}},
      {{-1.0f, 1.0f, 1.0f}, {0.327f, 0.483f, 0.844f}},
      {{1.0f, 1.0f, -1.0f}, {0.822f, 0.569f, 0.201f}},
      {{-1.0f, -1.0f, -1.0f}, {0.435f, 0.602f, 0.223f}},
      {{-1.0f, 1.0f, -1.0f}, {0.310f, 0.747f, 0.185f}},
      {{1.0f, -1.0f, 1.0f}, {0.597f, 0.770f, 0.761f}},
      {{-1.0f, -1.0f, -1.0f}, {0.559f, 0.436f, 0.730f}},
      {{1.0f, -1.0f, -1.0f}, {0.359f, 0.583f, 0.152f}},
      {{1.0f, 1.0f, -1.0f}, {0.483f, 0.596f, 0.789f}},
      {{1.0f, -1.0f, -1.0f}, {0.559f, 0.861f, 0.639f}},
      {{-1.0f, -1.0f, -1.0f}, {0.195f, 0.548f, 0.859f}},
      {{-1.0f, -1.0f, -1.0f}, {0.014f, 0.184f, 0.576f}},
      {{-1.0f, 1.0f, 1.0f}, {0.771f, 0.328f, 0.970f}},
      {{-1.0f, 1.0f, -1.0f}, {0.406f, 0.615f, 0.116f}},
      {{1.0f, -1.0f, 1.0f}, {0.676f, 0.977f, 0.133f}},
      {{-1.0f, -1.0f, 1.0f}, {0.971f, 0.572f, 0.833f}},
      {{-1.0f, -1.0f, -1.0f}, {0.140f, 0.616f, 0.489f}},
      {{-1.0f, 1.0f, 1.0f}, {0.997f, 0.513f, 0.064f}},
      {{-1.0f, -1.0f, 1.0f}, {0.945f, 0.719f, 0.592f}},
      {{1.0f, -1.0f, 1.0f}, {0.543f, 0.021f, 0.978f}},
      {{1.0f, 1.0f, 1.0f}, {0.279f, 0.317f, 0.505f}},
      {{1.0f, -1.0f, -1.0f}, {0.167f, 0.620f, 0.077f}},
      {{1.0f, 1.0f, -1.0f}, {0.347f, 0.857f, 0.137f}},
      {{1.0f, -1.0f, -1.0f}, {0.055f, 0.953f, 0.042f}},
      {{1.0f, 1.0f, 1.0f}, {0.714f, 0.505f, 0.345f}},
      {{1.0f, -1.0f, 1.0f}, {0.783f, 0.290f, 0.734f}},
      {{1.0f, 1.0f, 1.0f}, {0.722f, 0.645f, 0.174f}},
      {{1.0f, 1.0f, -1.0f}, {0.302f, 0.455f, 0.848f}},
      {{-1.0f, 1.0f, -1.0f}, {0.225f, 0.587f, 0.040f}},
      {{1.0f, 1.0f, 1.0f}, {0.517f, 0.713f, 0.338f}},
      {{-1.0f, 1.0f, -1.0f}, {0.053f, 0.959f, 0.120f}},
      {{-1.0f, 1.0f, 1.0f}, {0.393f, 0.621f, 0.362f}},
      {{1.0f, 1.0f, 1.0f}, {0.673f, 0.211f, 0.457f}},
      {{-1.0f, 1.0f, 1.0f}, {0.820f, 0.883f, 0.371f}},
      {{1.0f, -1.0f, 1.0f}, {0.982f, 0.099f, 0.879f}},
  };
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
  const std::vector<uint32_t> indices = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                         12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                         24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
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

Cube::~Cube() { log_func; }
