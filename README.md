# Pikzel

A simple graphics rendering engine.  To be clear, this is not a "game engine" - it does only graphics.  You can render a scene, move the camera around with keyboard/mouse, and tweak settings (via ImGui).  There is no audio, no physics, no route finding, no networking, etc. There will be a scene "editor" at some point.

This project is mainly a learning exercise for me, and I hope that the code is clear and simple enough to be accessible to other developers out there that are just starting on a graphics programing journey.

## Goals
- Provide a simple abstraction layer over different rendering APIs (such as OpenGL, Vulkan, etc.)
- Application developer is free to interact with the engine at a low level (e.g. write your own shaders, issue your own draw calls), or can use higher level "renderers" provided by the engine.  These will render a scene for you.
- Be accessible to beginners. To this end, a series of demo applications are included that pretty much follow along the [learnopengl.com](https://learnopengl.com) tutorials.

## Features
- [ ] Supported platforms
  - [x] Windows 10
  - [ ] Linux
  - [ ] Mac
  - [ ] Mobile

- [ ] Rendering APIs
  - [x] OpenGL
  - [x] Vulkan
  - [ ] DirectX
  - [ ] Metal

- [ ] "Renderers"
  - [ ] Rasterization
    - [x] Simple shaders
    - [x] Textures
    - [x] Lighting (directional/point lights)
    - [x] Very simple material (specular/roughness)
    - [x] "Mesh" renderer
    - [x] Render to offscreen framebuffer
    - [x] Skyboxes
    - [ ] Shadow mapping
      - [x] Directional light shadows
      - [x] Point light shadows
      - [ ] Cascades
      - [ ] Percent closer soft shadows (PCSS)
    - [ ] Normal maps
    - [ ] Bloom
    - [ ] Deferred rendering
    - [ ] Screen space ambient occulsion
    - [ ] Screen space reflection
    - [ ] Physically based rendering (PBR)

  - [ ] Ray traced
    - [ ] Vulkan (VK_KHR_ray_tracing)
    - [ ] Nvidia Optix
    - [ ] other (e.g. non-nvidia specific)

  - [ ] Application Framework
    - [x] Entry point
    - [x] Logging
    - [x] Tracy integration (performance profiling)
    - [x] Runtime load of selected rendering API
    - [x] Main window management
    - [x] Main loop
    - [x] Event system
    - [x] Basic ImGui integration

  - [ ] Scene editor

## Building
This project is C++ and uses CMake to generate build system files.  My development environment is Visual Studio 2019 on Win10.  Others are untested, but may work (with hopefully only minor changes).

### Prerequisites
- Vulkan SDK  (this is currently required even if you are using the OpenGL backend, as shaders are written in Vulkan GLSL dialect (and then cross compiled with Spir-V cross).  The project is currently using the Spir-V tools distributed with the Vulkan SDK rather than bringing them in via submodules and building them independently.  This will be changed in the future (so that use of OpenGL will not depend on Vulkan SDK)).
- All other dependencies are brought in via git submodules, so there is no need to pre-install anything.  The other dependencies are:
  - assimp  (asset (aka 3d models) importing)
  - cmrc    (for embedding resources (such as shader binaries) into the compiled application)
  - entt    (Entity Component System, plus this is also used for the event system, and compile time string hashing)
  - glfw    (Window management)
  - glm     (maths)
  - imgui   (gui components)
  - spdlog  (logging)
  - stb     (image file loading)
  - tracy   (performance profiling)

### Build
- ```git clone --recursive https://github.com/freeman40/Pikzel.git```
- Open cloned folder in Visual Studio
- Build (using Visual Studio built-in cmake support)

Be aware that the first time you build, it will take a little longer than usual (a couple of minutes maybe) as it builds all of the dependencies.

## Acknowledgements
- Some of the code in this project is based on The Cherno's [Game Engine Series](https://thecherno.com/engine) youtube channel
- The demos are mostly lifted directly from Joey de Vries' [learnopengl.com](https://learnopengl.com)
- The Vulkan renderer backend is inspired by Alexander Overvoorde's [Vulkan Tutorial](https://vulkan-tutorial.com)
