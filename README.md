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
* Vulkan SDK
* All other dependences are brought in via submodules

### Build
* git clone --recursive https://github.com/freeman40/Pikzel.git
* open folder in Visual Studio
* generate cmake cache (using Visual Studio built-in cmake support)
* build
