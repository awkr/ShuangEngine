cmake_minimum_required(VERSION 3.20)

# Project declaration
project(Shuang)

set(CMAKE_CXX_STANDARD 17)

# ======= Paths =======
# Where our external libs are
set(EXTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Sub-projects
add_subdirectory(${EXTERNAL}/glm)
add_subdirectory(${EXTERNAL}/glfw)
add_subdirectory(${EXTERNAL}/spdlog)
add_subdirectory(${EXTERNAL}/fmt)
add_subdirectory(shuang)
add_subdirectory(examples)
