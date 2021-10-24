# Project structure

1. shuang/: project library source code
2. third_party/: external libs

# Dependencies

Use `git submodule` to manage dependencies for simplicity. Other means such as CMake `FetchContent` can also do this,
but needs more refresh time when reconfiguring project.

## Libraries used

1. glm ( tag: 0.9.9.8 )
2. glfw ( tag: 3.3.4 )
3. spdlog ( tag: v1.9.2 )
4. fmt ( tag: 8.0.1 )

# TODO

1. Camera: use Quaternion
