# Renderer Foundation

This document describes the first layer of the renderer: getting a GPU device, presenting frames to a window, and the minimum set of resource types needed to draw. The `HelloTriangle` example uses all of it.

## Pieces

| Type | Header | Responsibility |
|---|---|---|
| `Window` | `Platform/Window.h` | A GLFW window with no graphics context. Hands out native OS handles. |
| `GraphicsDevice` | `Renderer/GraphicsDevice.h` | The NRI device, its interface tables and the graphics queue. No window needed. |
| `SwapChain`, `Frame` | `Renderer/SwapChain.h` | Back buffers, frames in flight, CPU/GPU synchronisation, resize handling. |
| `CommandList`, `RenderTarget` | `Renderer/CommandList.h` | Recording rendering passes, pipeline and vertex buffer binding, and draws. |
| `Buffer`, `BufferUsage` | `Renderer/Buffer.h` | A GPU buffer and its memory, filled with initial data. |
| `Pipeline`, `GraphicsPipelineSettings` | `Renderer/Pipeline.h` | A graphics pipeline and its (currently empty) pipeline layout. |
| `Shader` | `Renderer/Shader.h` | Loads SPIR-V compiled at build time. |
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

NRI creates 8-bit swap chains with a UNORM format, and enables `VK_KHR_swapchain_mutable_format`, which permits sRGB views of them. All desktop Vulkan drivers support that extension. The engine does not handle its absence.

### Checking it: the SrgbTest example

`examples/SrgbTest` draws a test pattern that looks visibly wrong if colors are blended or interpolated in sRGB space. It is based on the pattern from John Novak's article "What every coder should know about gamma", which Pikzel-original also used.

- **Grey ramp.** 21 squares in equal sRGB steps from black to white. This should look like an even ramp, with mid-grey (128) in the middle.
- **Blending.** A 50% transparent red bar crosses an opaque green bar. With linear blending its brightness is even along its length. Blending sRGB values would make it darker where it crosses the green bar.

Measured on screen, the transparent bar is (255, 188, 188) over white and (187, 188, 0) over green. Linear 0.5 encodes to sRGB 188, which confirms the blend happens in linear space. Blending in sRGB space would have produced 128.

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

## Shaders

Shaders are written in Slang and compiled to SPIR-V **at build time** by `pikzel_add_shaders()` (`cmake/Shaders.cmake`):

- Each `.slang` file becomes one `.spv` file containing all its entry points. With `-fvk-use-entrypoint-name`, entry points keep their names (for example `VertexMain`), and `GraphicsPipelineSettings` selects them by name.
- Output goes to `bin/Shaders/`, next to the executables. `Shader("Triangle")` loads `Shaders/Triangle.spv` relative to the executable, so the working directory does not matter.
- slangc writes a dependency file, so a shader is rebuilt when any module it imports changes.

Compiling at build time means shader errors are build errors, and startup pays no compile cost. Run-time compilation (for hot reload) can be added later using the Slang library.

### Vulkan binding offsets

HLSL-style registers come in four kinds: `b` (constant buffers), `t` (shader resources), `u` (read-write resources) and `s` (samplers). D3D12 keeps each kind in a separate namespace. Vulkan has a single binding namespace per descriptor set, so each kind is shifted into its own range (samplers from 0, constant buffers from 32, read-write from 64, shader resources from 128).

The shader compiler assigns those bindings, and NRI builds descriptor set layouts that expect them, so both must use the same offsets. The offsets are defined once in `cmake/Shaders.cmake`, passed to slangc as `-fvk-*-shift` flags, and passed to the engine as compile definitions for `DeviceCreationDesc::vkBindingOffsets`.

## Current limitations and next steps

These are deliberate omissions, to be filled in as real requirements arrive:

- **Vulkan only.** NRI's D3D12 backend is switched off, because it needs DXIL shaders, and Slang can only produce DXIL with `dxcompiler.dll` alongside it. Enabling D3D12 means turning the backend on in `cmake/Dependencies.cmake`, compiling a second shader target, and choosing the API at device creation.
- **Synchronous uploads.** `Buffer`'s initial upload waits for the GPU. Streaming and per-frame data will need upload buffers in host-visible memory, or NRI's streamer.
- **One allocation per buffer.** Fine for a handful of buffers. A real scene will want sub-allocation (NRI can route allocations through VMA).
- **No depth buffer, textures, or descriptors yet.** These come next, along with constant buffers and a camera.
- **One vertex buffer, one color target, two blend modes.** `GraphicsPipelineSettings` covers exactly what is used so far: `BlendMode::Opaque` and `BlendMode::Alpha` (standard "over" compositing). Index buffers, depth, other blend modes and multiple render targets get settings as they are needed.
- **Vulkan vertex locations only.** Vertex attributes are matched to shader inputs by location. A D3D backend would also need HLSL semantic names.
- **SDR output only.** The swap chain is 8-bit sRGB (`BT709_G22_8BIT`). Linear values above 1.0 are clipped, not tone-mapped. HDR rendering targets and tone mapping will come with the lighting work.
- **Swap chain format assumed stable.** Pipelines are created with the format reported at startup. If recreation ever produced a different format (it does not in practice for the same window and display), those pipelines would need rebuilding.

## Validation

In debug builds `GraphicsDevice` enables both NRI's validation and the Vulkan validation layers. The Vulkan layers come with the [Vulkan SDK](https://vulkan.lunarg.com/); without the SDK, the device is still created, just without API validation. All validation output goes to the engine log, so a validation error shows up as a `CORE` error line.
