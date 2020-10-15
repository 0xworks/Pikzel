# Pikzel

Yet another "engine" for creating applications to render something.
There are hundreds of these already out there, so why create another one?

Because I want to, and that's it.

## Intended Features
- [x] Provide endless hours of coding fun for me.  I might even learn something.
- [ ] Engine should allow for both "online" (interactive rendering to a window) and "offline" (render to a file, no windows involved).
- [x] Engine should facilitate writing client-side code that is rendering-backend-agnostic.  It should be possible to switch the rendering-backend without any changes to the client code.  e.g. if you want to render a scene using Vulkan instead of OpenGL, or using ray-tracing instead of rasterization.
- [ ] Provide support client apps to build "tooling" UI via ImGui
- [ ] Provide some rendering backends
  - [x] Open GL (rasterization)
  - [x] Vulkan (rasterization)
  - [ ] Vulkan (path tracing)
  - [ ] Optix (path tracing)
  - [ ] CPU (offline render only) (unlikely, as I'd only resort to this if there is something I cannot figure out how to do using the GPU)
  - [ ] DirectX (unlikey, as I am not really interested in learning this)

## Building
This project is C++ and uses the cmake to generate build system files.  My development environment is Visual Studio 2019 on Windows.  Others are untested, but may work (with hopefully minor changes).

### Prerequisites
* Vulkan SDK  (this is currently required even if you are using the OpenGL backend, as shaders are written in Vulkan GLSL dialect (and then cross compiled with Spir-V cross).  The project is currently using the SpirV tools distributed with the Vulkan SDK rather than bringing them in via submodules and building them independently.  This will be changed in the future. (so that use of OpenGL will not depend on Vulkan SDK)).
* All other dependencies are brought in via submodules.  The other dependencies are:
  * assimp  (asset (aka 3d models) importing)
  * cmrc    (for embedding resources (such as shader binaries) into the compiled application)
  * entt    (Entity Component System, plus this is also used for the event system, and compile time string hashing)
  * glfw    (Window management)
  * glm     (maths)
  * imgui   (gui components)
  * spdlog  (logging)
  * stb     (image file loading)
  * tracy   (performance profiling)

### Build
* git clone --recursive https://github.com/freeman40/Pikzel.git
* open folder in Visual Studio
* generate cmake cache (using Visual Studio built-in cmake support)
* build

Be aware that the first time you build, it will take a little longer than usual (a couple of minutes maybe) as it builds the dependecies also (in particular, assimp).
