# Renderer Foundation

This document describes the first layer of the renderer: getting a GPU device, presenting frames to a window, and the minimum set of resource types needed to draw. The `HelloTriangle` example uses all of it.

## Pieces

| Type | Header | Responsibility |
|---|---|---|
| `Window` | `Platform/Window.h` | A GLFW window with no graphics context. Hands out native OS handles. |
| `GraphicsDevice`, `GraphicsApi` | `Renderer/GraphicsDevice.h` | The NRI device for the chosen graphics API, its interface tables and the graphics queue. No window needed. |
| `SwapChain`, `Frame` | `Renderer/SwapChain.h` | Back buffers, frames in flight, CPU/GPU synchronisation, resize handling. |
| `CommandList`, `RenderTarget` | `Renderer/CommandList.h` | Recording rendering passes, pipeline and vertex buffer binding, and draws. |
| `Buffer`, `BufferUsage` | `Renderer/Buffer.h` | A GPU buffer and its memory, filled with initial data. |
| `Pipeline`, `GraphicsPipelineSettings` | `Renderer/Pipeline.h` | A graphics pipeline and its (currently empty) pipeline layout. |
| `Shader` | `Renderer/Shader.h` | Loads one entry point compiled at build time (SPIR-V or DXIL, for the device's API). |
| `Format` | `Renderer/Format.h` | The texture and vertex formats the engine uses. |
| `Color` | `Renderer/Color.h` | A linear RGBA color, with conversions from sRGB values and hex codes. |
| `NriInterface`, `CheckNri`, conversions | `Renderer/Nri.h` | **Internal.** NRI includes, interface tables, error handling, enum conversions. |

Dependencies point one way: the renderer knows about `Window` (to build a swap chain), but `Window` knows nothing about the renderer or NRI. It exposes a `NativeWindowHandle` (a variant of Win32, X11 and Wayland handles), and `SwapChain` converts that into NRI's window description.

`GraphicsDevice` does not need a window, so off-screen GPU work (for example in tools) does not require one. A program that does no rendering at all simply never creates a `GraphicsDevice`.

## NRI is an implementation detail

Code that uses the renderer (examples, games, and later the editor) never sees NRI. The renderer's public headers use engine types (`Format`, `BufferUsage`, `GraphicsPipelineSettings`, `CommandList`) and only *forward-declare* the NRI types they hold pointers to. NRI's own headers are included only by the renderer's `.cpp` files, through `Renderer/Nri.h`. CMake links NRI to Pikzel `PRIVATE`, so programs do not even get NRI's include paths. `GraphicsDevice` holds its NRI interface tables through a `std::unique_ptr` for the same reason: a by-value member would need the full definition in the header.

This boundary has costs and benefits:

- **Benefit:** the public API describes only what the engine supports, in the engine's naming conventions. For example, `Format` lists a handful of formats, not NRI's hundred-plus.
- **Benefit:** renderer users are insulated from NRI version changes, which have historically been frequent.
- **Cost:** each NRI concept the engine exposes needs an engine type and a conversion function (see `ToNri`/`FromNri`). These are deliberately small and only grow when a feature needs them.

Renderer classes still have a few accessors that return NRI objects (`GraphicsDevice::GetNri()`, `Buffer::GetNriBuffer()`, ...), marked "Renderer internal". Other renderer classes need them to work together. They are harmless to everyone else, because without NRI's headers nothing can be done with the forward-declared types.

The engine classes also take on what NRI leaves to the caller:

- **Lifetime.** Every class that owns NRI objects destroys them in its destructor. `Buffer` and `Pipeline` are move-only. `GraphicsDevice` and `SwapChain` cannot be moved, because other objects refer to them.
- **Memory.** `Buffer` creates the buffer, allocates and binds memory, uploads the initial data, and frees everything.
- **Synchronisation.** `SwapChain` handles frames in flight, semaphores, fences and back buffer layout transitions.
- **Errors.** `CheckNri` turns a failed `nri::Result` into a `std::runtime_error` naming the operation. NRI's diagnostic messages, including validation errors, go to the engine log.

`CommandList` is a lightweight, non-owning view of a command buffer owned by `SwapChain`. `BeginRendering()` also sets the viewport and scissor to cover the whole target, since nearly every pass wants that. `BindVertexBuffer()` takes the vertex stride from the bound pipeline, so the stride is stated once, in `GraphicsPipelineSettings`.

## Color spaces

The renderer works with **linear** color values everywhere: in `Color`, in vertex data, in shaders, and in clear colors. Displays expect **sRGB-encoded** values. The conversion happens in hardware, at the very end.

### Why it matters

sRGB encoding is roughly a power curve (gamma ~2.2) that spends more of the available values on dark tones, where human vision is most sensitive. The value 0.5 in sRGB emits only about 21% as much light as 1.0. Arithmetic on encoded values is therefore physically wrong. Interpolating halfway between sRGB red `(1, 0, 0)` and green `(0, 1, 0)` gives `(0.5, 0.5, 0)`, a dim olive. Doing the same in linear space and then encoding gives roughly `(0.74, 0.74, 0)`, the bright yellow you see when red and green light mix. The same error affects lighting, alpha blending, texture filtering and mipmapping.

### How the engine does it

- **Input.** `Color` always holds linear values. Colors from pickers, image editors and hex codes are sRGB, so they are converted on the way in: `Color::FromSrgbHex(0xFF8000)`, `Color::FromSrgb8(255, 128, 0)` or `Color::FromSrgb(1.0f, 0.5f, 0.0f)`. These use the exact piecewise sRGB transfer function, not a gamma-2.2 approximation. Alpha is never encoded. Writing `Color{r, g, b, a}` directly means the values are already linear.
- **Range.** sRGB components and alpha are only defined on [0, 1]. Out-of-range input (including NaN) is a programmer error: it asserts in debug builds and is clamped in release builds. Linear values in a directly constructed `Color` are not checked, because HDR values above 1.0 are legitimate.
- **Output.** `SwapChain` renders through an sRGB view (`RGBA8_SRGB`) of its back buffers. The GPU encodes each linear value as it writes it, after blending, so blending also happens in linear space. `SwapChain::GetFormat()` returns this sRGB format, so pipelines are created to match.
- **Shaders** therefore never encode or decode colors themselves.

NRI creates 8-bit swap chains with a UNORM format and renders through sRGB views of them. On Vulkan this needs `VK_KHR_swapchain_mutable_format`, which all desktop drivers support; the engine does not handle its absence. D3D12 permits it natively: flip-model swap chains cannot have an sRGB format, but an sRGB render target view of their UNORM back buffers is allowed.

### Checking it: the SrgbTest example

`examples/SrgbTest` draws a test pattern that looks visibly wrong if colors are blended or interpolated in sRGB space. It is based on the pattern from John Novak's article "What every coder should know about gamma", which Pikzel-original also used.

- **Grey ramp.** 21 squares in equal sRGB steps from black to white. This should look like an even ramp, with mid-grey (128) in the middle.
- **Blending.** A 50% transparent red bar crosses an opaque green bar. With linear blending its brightness is even along its length. Blending sRGB values would make it darker where it crosses the green bar.

Measured on screen, the transparent bar is (255, 188, 188) over white and (187, 188, 0) over green, on both Vulkan and D3D12. Linear 0.5 encodes to sRGB 188, which confirms the blend happens in linear space. Blending in sRGB space would have produced 128.

Textures will follow the same rule when they arrive: color textures (albedo, UI) use `*Srgb` formats so that sampling returns linear values, and data textures (normals, roughness) use `*Unorm`.

## Frames in flight

The CPU records frame N+1 while the GPU executes frame N. Anything the CPU writes during a frame — currently the command allocator and command buffer — exists once per frame in flight (two by default).

```
CPU:  record 0 | record 1 | wait(0 done) record 2 | wait(1 done) record 3 | ...
GPU:           | execute 0          | execute 1              | execute 2  ...
```

A single timeline fence tracks progress. Frame N signals it with the value N + 1 when its GPU work completes. Before reusing the resources of frame N, `BeginFrame()` waits for the fence to reach `N - framesInFlight + 1`, meaning the frame that last used those resources has finished.

## Swap chain synchronisation

Each frame uses two binary semaphores on the GPU:

1. **Acquire semaphore**: signalled when the presentation engine has finished with the back buffer. The frame's submission waits on it, but only at the color-attachment stage, so vertex work can start early.
2. **Release semaphore**: signalled when rendering finishes. Presentation waits on it.

The release semaphore belongs to the back buffer, because presenting that back buffer consumes it. The acquire semaphore is needed *before* the back buffer index is known, so acquire semaphores are cycled by frame number instead. There is one more back buffer than frames in flight, which guarantees an acquire semaphore is never reused while still pending. This is the scheme NRI documents in `Extensions/NRISwapChain.h`.

Between `BeginFrame()` and `EndFrame()`, the back buffer is moved from `UNDEFINED` (its old contents are discarded) to `COLOR_ATTACHMENT`, and then to `PRESENT`.

## Resizing and minimising

`BeginFrame()` compares the window's framebuffer size with the swap chain's size every frame, and recreates the swap chain if they differ. It also recreates the swap chain when acquire or present reports `OUT_OF_DATE`. Recreation waits for the GPU to go idle first. That is slow, but it only happens on resize, and it guarantees nothing is still using the old back buffers.

While the window is minimised, its framebuffer size is zero and there is nothing to render into. `BeginFrame()` returns no frame, and `HelloTriangle` blocks in `Window::WaitEvents()` instead of spinning.

## Graphics APIs

The renderer runs on **Vulkan** everywhere, and on **D3D12** on Windows when the engine is configured with `PKZL_ENABLE_D3D12=ON`. With that option, the engine library contains both NRI backends and every shader is compiled for both. The API is chosen **at run time**, when the `GraphicsDevice` is created:

```cpp
pk::GraphicsDevice device({.api = pk::GraphicsApi::D3D12});
```

The examples take `--d3d12` on the command line. Asking for an API that was not compiled in throws a `std::runtime_error` that says so. `IsGraphicsApiAvailable()` checks this in advance.

Choosing at run time costs nothing extra, because NRI dispatches every call through its interface tables anyway. It means one build can compare both APIs, which is most of the point of having two. The option is off by default because it adds a download (the DirectX Shader Compiler, plus NRI's Agility SDK package) and roughly doubles shader build time.

Very little of the renderer is API-specific, because NRI hides the differences. What remains:

- **Shader bytecode.** Vulkan takes SPIR-V; D3D12 takes DXIL. `Shader` loads the right file for the device's API (see below).
- **Vertex attribute matching.** Vulkan matches vertex attributes to shader inputs by *location* (`[[vk::location(n)]]`); D3D12 by HLSL *semantic* (`: POSITION`, `: COLOR`). Each `VertexAttribute` therefore gives both, and `Pipeline` rejects an attribute with no semantic name on either API, so a mistake shows up on Vulkan too rather than only on the first D3D12 run.
- **Binding offsets.** Vulkan needs register kinds shifted into separate ranges (see below). D3D12 does not.

Clip space is the same on both: +Y is up. NRI's Vulkan backend flips its viewports (a negative viewport height) to match D3D, so shaders and vertex data need no per-API changes.

### The D3D12 build

- NRI's D3D12 backend is written against the **Agility SDK** headers, which are newer than the Windows SDK's, so NRI's Agility support stays enabled and NRI downloads the package. Programs only *run* on the Agility runtime if their executable exports `D3D12SDKVersion` and `D3D12SDKPath`. Pikzel's examples do not, so they use the D3D12 runtime that comes with Windows. NRI then logs that a newer feature query (`options22`) failed; this is expected and harmless. NRI still copies the unused Agility runtime into `bin/AgilitySDK/`.
- NRI's NVIDIA (NVAPI) and AMD (AGS) extension libraries are switched off. The engine does not use vendor-specific features.

## Shaders

Shaders are written in Slang and compiled **at build time** by `pikzel_add_shader()` (`cmake/Shaders.cmake`):

```cmake
pikzel_add_shader(HelloTriangle Triangle.slang ENTRY_POINTS VertexMain FragmentMain)
```

- Each entry point is compiled into a file of its own: `Triangle.VertexMain.spv` (SPIR-V, for Vulkan) and, when D3D12 is enabled, `Triangle.VertexMain.dxil`. slangc takes each entry point's stage from its `[shader("...")]` attribute.
- Output goes to `bin/Shaders/`, next to the executables. `Shader(device, "Triangle", "VertexMain")` loads the file for the device's API from there, relative to the executable, so the working directory does not matter. `GraphicsPipelineSettings` takes one `Shader` per stage.
- slangc writes a dependency file, so a shader is rebuilt when any module it imports changes.

**Why one file per entry point.** SPIR-V can hold several entry points in one module, but D3D12 cannot use DXIL that way: a DXIL file with several entry points is a *library*, which only ray tracing and work graph pipelines accept. A graphics pipeline needs one compiled shader per stage. Using the same one-file-per-entry-point layout for SPIR-V keeps `Shader` the same on both APIs. The cost is listing the entry points in CMake.

**DXIL needs DXC.** Slang does not generate DXIL itself. It translates to HLSL and hands that to the DirectX Shader Compiler's `dxcompiler.dll`, which the Slang package does not include. A pinned DXC release is downloaded (`cmake/Dependencies.cmake`) and passed to slangc with `-dxc-path`. The package also contains `dxil.dll`, which signs the DXIL; D3D12 rejects unsigned shaders. The Windows SDK has its own copy of DXC, but its version depends on which SDK is installed, whereas a pinned download gives the same output on every machine. DXIL targets shader model 6.6 (`PKZL_D3D12_SHADER_MODEL`).

Compiling at build time means shader errors are build errors, and startup pays no compile cost. Run-time compilation (for hot reload) can be added later using the Slang library.

### Vulkan binding offsets

HLSL-style registers come in four kinds: `b` (constant buffers), `t` (shader resources), `u` (read-write resources) and `s` (samplers). D3D12 keeps each kind in a separate namespace. Vulkan has a single binding namespace per descriptor set, so each kind is shifted into its own range (samplers from 0, constant buffers from 32, read-write from 64, shader resources from 128).

The shader compiler assigns those bindings, and NRI builds descriptor set layouts that expect them, so both must use the same offsets. The offsets are defined once in `cmake/Shaders.cmake`, passed to slangc as `-fvk-*-shift` flags (for SPIR-V only), and passed to the engine as compile definitions for `DeviceCreationDesc::vkBindingOffsets`, which only NRI's Vulkan backend reads.

## Current limitations and next steps

These are deliberate omissions, to be filled in as real requirements arrive:

- **Vulkan and D3D12 only.** NRI also has D3D11, Metal and WebGPU backends. D3D11 lacks the modern features the renderer is aiming for. Metal and WebGPU would need new shader targets and platforms the engine does not target yet.
- **Synchronous uploads.** `Buffer`'s initial upload waits for the GPU. Streaming and per-frame data will need upload buffers in host-visible memory, or NRI's streamer.
- **One allocation per buffer.** Fine for a handful of buffers. A real scene will want sub-allocation (NRI can route allocations through VMA).
- **No depth buffer, textures, or descriptors yet.** These come next, along with constant buffers and a camera.
- **One vertex buffer, one color target, two blend modes.** `GraphicsPipelineSettings` covers exactly what is used so far: `BlendMode::Opaque` and `BlendMode::Alpha` (standard "over" compositing). Index buffers, depth, other blend modes and multiple render targets get settings as they are needed.
- **Vertex layouts are not checked against shaders.** Locations and semantic names in `VertexAttribute` must match the shader by hand. Slang's reflection API could verify them when a pipeline is created.
- **SDR output only.** The swap chain is 8-bit sRGB (`BT709_G22_8BIT`). Linear values above 1.0 are clipped, not tone-mapped. HDR rendering targets and tone mapping will come with the lighting work.
- **Swap chain format assumed stable.** Pipelines are created with the format reported at startup. If recreation ever produced a different format (it does not in practice for the same window and display), those pipelines would need rebuilding.

## Validation

In debug builds `GraphicsDevice` enables both NRI's validation and the graphics API's own validation. On Vulkan that is the validation layers, which come with the [Vulkan SDK](https://vulkan.lunarg.com/). On D3D12 it is the debug layer, which comes with the Windows **Graphics Tools** optional feature (Settings > System > Optional features). Without them, the device is still created, just without API validation. All validation output goes to the engine log, so a validation error shows up as a `CORE` error line.
