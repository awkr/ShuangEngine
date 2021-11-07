# Project structure

1. shuang/: project library source code
2. third_party/: external libs

# Dependencies

Use `git submodule` to manage dependencies for simplicity. Other means such as CMake `FetchContent` can also do this,
but needs more refresh time when reconfiguring project.

# User manual

## Camera movement & rotation

1. `W`, `S`, mouse scroll: move forward and back
2. `A`, `D`, mouse moves horizontally with right button: move left and right
3. `R`, `F`, mouse moves vertically with right button: move up and down
4. `Q`, `E`: roll
5. `Up`, `Down`, mouse moves vertically with left button: pitch
6. `Left`, `Right`, mouse moves horizontally with left button: yaw

## Libraries used

1. glm ( tag: 0.9.9.8 )
2. glfw ( tag: 3.3.4 )
3. spdlog ( tag: v1.9.2 )
4. fmt ( tag: 8.0.1 )

# [TODO]

1. 抽象出 Timer 类
