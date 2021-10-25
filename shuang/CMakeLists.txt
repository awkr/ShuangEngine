file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cc")

set(LIB_NAME shuang)
set(PUBLIC_HDR_DIR include)

# Specify where our headers are
include_directories(${PUBLIC_HDR_DIR})

add_library(${LIB_NAME} STATIC ${HDRS} ${SRCS})

# Specify where the public headers of this library are
target_include_directories(${LIB_NAME} PUBLIC ${PUBLIC_HDR_DIR})

# ======= Dependencies =======
# 因外部项目引用 libshuang 时，有可能（或不反对）调用 glm 或 spdlog，故设置为 PUBLIC。
# 相反，外部不应直接调用 GLFW 相关 API，即 GLFW 应被封装在内部，故设置为 PRIVATE。
target_link_libraries(${LIB_NAME} PUBLIC glm spdlog fmt)
target_link_libraries(${LIB_NAME} PRIVATE glfw)

# 因基于 Vulkan 的程序实际运行时，需要宿主机装有 Vulkan SDK。
# 故我们应直接使用宿主机环境中的 header 与 library，比规避版本不一致的问题。
# see https://stackoverflow.com/questions/56795645/how-can-i-add-vulkan-to-a-cross-platform-cmake-project
find_package(Vulkan REQUIRED FATAL_ERROR)
target_link_libraries(${LIB_NAME} PRIVATE ${Vulkan_LIBRARY})
target_include_directories(${LIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
