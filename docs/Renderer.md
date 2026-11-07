# Renderer Foundation

This document describes the first layer of the renderer: getting a GPU device, presenting frames to a window, and the resource types needed to draw textured 3D geometry. `HelloTriangle` is the smallest program using it; `TexturedCubes` uses all of it.

## Pieces

| Type | Header | Responsibility |
|---|---|---|
| `Window` | `Platform/Window.h` | A GLFW window with no graphics context. Hands out native OS handles. |
| `GraphicsDevice`, `GraphicsApi` | `Renderer/GraphicsDevice.h` | The NRI device for the chosen graphics API, its interface tables, the graphics queue and the descriptor pool. No window needed. |
| `SwapChain`, `Frame` | `Renderer/SwapChain.h` | Back buffers, an optional depth buffer, frames in flight, CPU/GPU synchronisation, resize handling. |
| `CommandList`, `RenderTarget`, `RenderPassSettings` | `Renderer/CommandList.h` | Recording rendering passes, binding pipelines, buffers, descriptor sets and push constants, and draws. |
| `Buffer`, `BufferSettings` | `Renderer/Buffer.h` | A GPU buffer and its memory: device-local and filled once, or host-writable and rewritten each frame. |
| `Texture`, `TextureSettings` | `Renderer/Texture.h` | A 2D texture, its memory and its views: sampled by shaders, rendered into, or used as a depth buffer. |
| `Sampler` | `Renderer/Sampler.h` | Filtering and addressing state for reading textures. |
| `Pipeline`, `GraphicsPipelineSettings` | `Renderer/Pipeline.h` | A graphics pipeline and its pipeline layout (descriptor sets and push constants). |
| `DescriptorSet` | `Renderer/DescriptorSet.h` | A group of resource bindings matching one of a pipeline's descriptor set layouts. |
| `Shader` | `Renderer/Shader.h` | Loads one entry point compiled at build time (SPIR-V or DXIL, for the device's API). |
| `Float4x4`, `ToFloat4x4` | `Renderer/ShaderTypes.h` | Fixed-layout types that mirror shader data, converted explicitly from RTM. |
| `CreateLookAtMatrix`, `CreatePerspectiveMatrix` | `Renderer/Camera.h` | View and projection matrices in the engine's coordinate conventions. |
| `Format` | `Renderer/Format.h` | The texture, depth and vertex formats the engine uses. |
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

- **Lifetime.** Every class that owns NRI objects destroys them in its destructor. `Buffer`, `Texture`, `Sampler` and `Pipeline` are move-only. `GraphicsDevice` and `SwapChain` cannot be moved, because other objects refer to them.
- **Memory.** `Buffer` and `Texture` create the resource, allocate and bind memory, create the views they need, upload any initial data, and free everything.
- **Synchronisation.** `SwapChain` handles frames in flight, semaphores, fences and back buffer layout transitions.
- **Errors.** `CheckNri` turns a failed `nri::Result` into a `std::runtime_error` naming the operation. NRI's diagnostic messages, including validation errors, go to the engine log.

`CommandList` is a lightweight, non-owning view of a command buffer owned by `SwapChain`. `BeginRendering()` also sets the viewport and scissor to cover the whole color target, since nearly every pass wants that. `BindVertexBuffer()` takes the vertex stride from the bound pipeline, so the stride is stated once, in `GraphicsPipelineSettings`.

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

Textures follow the same rule: color textures (albedo, UI) use `*Srgb` formats so that sampling returns linear values, and data textures (normals, roughness) use `*Unorm`. Texel data for an sRGB texture is stored as sRGB-encoded bytes, exactly as an image editor produces them, with no conversion on the CPU: `TexturedCubes` builds its checkerboard from sRGB hex codes this way. Because the GPU decodes before it filters, bilinear filtering also happens in linear space.

## Coordinate conventions

The renderer's 3D conventions are chosen to match glTF, the engine's main asset format, so imported meshes and scenes need no conversion:

- **Right-handed, +Y up.** A camera looks down its -Z axis, with +X to its right.
- **Counter-clockwise front faces.** A triangle whose vertices appear counter-clockwise on screen faces the viewer. `CullMode::Back` discards the others.
- **Clip space as in D3D.** x and y from -1 to +1 with +y up, depth from 0 to 1. NRI flips Vulkan's viewport, so this holds on both APIs.
- **Reversed-Z depth**, described below.

RTM's own camera helpers (`rtm::view_look_at`, `rtm::proj_perspective_fov`) are left-handed and use traditional depth, so the engine builds its view and projection matrices itself, in `Renderer/Camera.h`. Each is a few lines, and the unit tests in `tests/Renderer/CameraTests.cpp` pin down the conventions: where the eye, target and up direction land in view space, and the depth of points at the near plane and far away.

### Reversed-Z

`CreatePerspectiveMatrix()` maps the near plane to depth **1** and infinitely distant points to depth **0**, the opposite of the traditional mapping.

Perspective projection makes stored depth proportional to 1/distance, so most of the depth range is spent close to the near plane. Floating-point numbers are also unevenly spread, with far more representable values near 0 than near 1. With traditional depth (near = 0), these two effects pile up at the same end, and distant geometry is left with very little precision, causing z-fighting. Reversing the range makes them roughly cancel out, giving almost uniform precision at every distance with a 32-bit float depth buffer (`Format::D32Float`). Precision is good enough to put the far plane at infinity, which removes a parameter that would otherwise need tuning per scene.

The rest of the renderer only has to agree on two things: depth buffers are cleared to 0 (the far plane), and the depth test passes for *greater* values (`CompareOp::Greater`, the default in `GraphicsPipelineSettings`).

## Matrices and data shared with shaders

CPU math types cannot simply be copied into GPU buffers. RTM's types are laid out for SIMD registers, while shaders read buffers according to their own layout rules. Data shared with shaders is therefore declared with plain C++ structs that mirror the Slang declarations, using types from `Renderer/ShaderTypes.h`, and filled by explicit conversion.

**Matrices.** RTM uses row vectors: a point is transformed as `p * M`, and a matrix stores its four rows (`x_axis`, `y_axis`, `z_axis`, `w_axis`) one after another, with the translation in the last row. Engine shaders use the same convention, so a matrix means the same thing on both sides:

- shaders transform with `mul(vector, matrix)`, and compose matrices left to right in the order they apply (`p * model * viewProjection`);
- shaders are compiled with `-matrix-layout-row-major` (`cmake/Shaders.cmake`), so a `float4x4` in a buffer is read one row at a time;
- `ToFloat4x4()` writes an RTM matrix's rows in that order.

No transposes are needed anywhere. The alternative, column vectors in shaders (`mul(matrix, vector)`), is equally valid but would put a transpose at the CPU/GPU boundary, and the CPU and shader code would then read differently.

**Packing.** Constant buffers and push constants use HLSL's packing rules: members are laid out in order in 16-byte rows, and a member may not straddle a row boundary. A `float3` followed by a `float` share one row; two `float3`s do not. C++ structs that mirror shader data `static_assert` their size and member offsets, so a mismatch is a compile error rather than garbage on screen (see `FrameConstants` in `examples/TexturedCubes/Main.cpp`).

The layouts are still checked by hand on the shader side. Slang's reflection API could verify them against the compiled shaders when a pipeline is created; that is a likely next step once the engine links the Slang library.

## Resource binding

Shaders reach everything except vertex data through the **pipeline layout**, which `GraphicsPipelineSettings` describes. It has two parts.

**Descriptor sets.** A descriptor is a small GPU-readable record saying where a resource is and how to read it: a texture view, a constant buffer view, a sampler. A descriptor set is a block of descriptors bound with one command. Each `DescriptorSetLayout` lists the ranges of registers its set covers, in one register space:

```cpp
.descriptorSets = {
   {.registerSpace = 1, .ranges = {{.type = DescriptorType::ConstantBuffer, .baseRegister = 0}}},
   {.registerSpace = 2, .ranges = {{.type = DescriptorType::Texture, .baseRegister = 0}, {.type = DescriptorType::Sampler, .baseRegister = 0}}},
},
```

which matches these shader declarations:

```slang
ConstantBuffer<FrameConstants> g_Frame : register(b0, space1);
Texture2D g_Albedo : register(t0, space2);
SamplerState g_Sampler : register(s0, space2);
```

On D3D12 the register space is part of the register name. On Vulkan, Slang turns `spaceN` into descriptor set N, and the register kinds are shifted into separate binding ranges (see [Vulkan binding offsets](#vulkan-binding-offsets)). NRI builds the matching layouts for each API.

Resources are grouped into sets by how often they change. `TexturedCubes` has a per-frame set (camera and light) and a per-material set (texture and sampler). Each set is bound only when its contents change, and a material's set is written once and reused every frame.

**Push constants.** A small block of data (at most 128 bytes is portable) written straight into the command buffer with `CommandList::PushConstants()`, with no buffer or descriptor involved. That makes them the cheapest way to pass data that changes with every draw, such as a model matrix. In Slang the block is declared `[[vk::push_constant]] ConstantBuffer<T> name : register(b0, space0);`. Space 0 is reserved for it when a pipeline uses push constants, and `Pipeline` rejects descriptor sets that use space 0 as well. (D3D12 calls these root constants, and Vulkan, push constants.)

**The descriptor pool.** Descriptor sets are allocated from a single pool, owned by `GraphicsDevice`. A single pool is needed because D3D12 can only have one shader-visible descriptor heap of each kind bound at a time, NRI backs a pool with such heaps, and so all sets used in one command buffer must come from the same pool. `SwapChain` attaches it to each frame's command buffer. NRI never returns individual sets to the pool, so sets are created up front (per material, per frame in flight), not per draw. The pool's capacity is fixed in `GraphicsDevice.cpp`.

This is the classic "bind a set per draw" model. A renderer aiming at GPU-driven rendering will likely move to **bindless** resources, where shaders index one large array of descriptors (`ResourceDescriptorHeap` in shader model 6.6, which is why D3D12 shaders target it). Descriptor sets remain the clearer way to learn how binding works, and are enough until the renderer draws many different materials.

## Per-frame data

Data the CPU rewrites every frame, such as the camera, lives in **host-writable** buffers (`BufferSettings::isHostWritable`). These are allocated in NRI's `DEVICE_UPLOAD` memory: video memory the CPU can write directly on GPUs with resizable BAR, falling back to system memory otherwise. `Buffer::Write()` copies into it straight away.

The CPU must not overwrite data the GPU is still reading. With two frames in flight, while the CPU writes frame N's constants the GPU may still be drawing frame N-1, so anything rewritten per frame exists once per frame in flight. `Frame::inFlightIndex` says which copy to use, and `SwapChain` guarantees that the GPU has finished with the previous frame that used the same index. `TexturedCubes` keeps one constant buffer and one descriptor set per frame in flight, and picks them with `inFlightIndex`.

Device-local buffers and textures created with initial data are filled once by NRI's `UploadData` helper, which copies through a staging buffer and waits for the GPU. That is fine at load time but too slow per frame.

## Depth buffer

`SwapChainSettings::depthFormat` asks the swap chain to keep a depth buffer the size of the window, recreated with the back buffers on resize. `Frame::depthBuffer` hands it out as a `RenderTarget`, and `RenderPassSettings` attaches it to a pass, normally cleared to 0.

There is one depth buffer, not one per back buffer. Frames execute one after another on the single graphics queue, and each frame clears the depth buffer, so sharing is safe as long as one frame's depth writes finish before the next frame's begin. `SwapChain::BeginFrame()` records a barrier for exactly that (a write-after-write dependency) at the start of each frame. A newly created depth buffer has no previous writes, and moves from the `UNDEFINED` layout instead.

Keeping the depth buffer in `SwapChain` means applications never deal with resizing it. When the renderer moves to off-screen rendering (HDR targets and tone mapping), those targets will own their own depth buffers, and this option will matter less.

## Frames in flight

The CPU records frame N+1 while the GPU executes frame N. Anything the CPU writes during a frame — the command allocator and command buffer, and any per-frame data the application keeps (see [Per-frame data](#per-frame-data)) — exists once per frame in flight (two by default).

```
CPU:  record 0 | record 1 | wait(0 done) record 2 | wait(1 done) record 3 | ...
GPU:           | execute 0          | execute 1              | execute 2  ...
```

A single timeline fence tracks progress. Frame N signals it with the value N + 1 when its GPU work completes. Before reusing the resources of frame N, `BeginFrame()` waits for the fence to reach `N - framesInFlight + 1`, meaning the frame that last used those resources has finished.

### Frame pacing

The fence keeps the CPU from running too far ahead of the GPU, but it does not tie the CPU to the display. On Vulkan with NVIDIA's Windows driver, measured in `TexturedCubes`, the presentation engine released back buffers in bursts: the CPU would block for two to four refreshes (33–66 ms), then record several frames almost instantly. Each frame was still displayed one refresh apart, but the animation time sampled when each frame was recorded was unevenly spaced, so motion visibly jittered every second or two. D3D12 happened not to show it, but nothing guaranteed that either.

The fix is a **waitable swap chain**. `BeginFrame()` first waits until frame `N - framesInFlight` has actually been presented, with `VK_KHR_present_wait` on Vulkan (each frame is presented with ID N + 1) and DXGI's frame latency waitable object on D3D12. The CPU then starts every frame in step with the display: in the same measurement, the longest frame dropped from 66 ms to about 17–22 ms at a 60 Hz refresh, on both APIs and with or without validation. The remaining long frames are at start-up and when the window is resized, where the swap chain is recreated.

The two APIs wait differently. Vulkan waits for a specific present ID, which must already have been queued on the current swap chain, so the first `framesInFlight` frames after the swap chain is created skip the wait. D3D12's latency object ignores the ID, starts out allowing `framesInFlight` queued frames, and must be waited on every frame from the first; skipping waits would let extra frames queue up and add latency.

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
- **Synchronous loading.** Initial uploads of buffers and textures wait for the GPU. Streaming assets in while running will need asynchronous uploads (NRI's streamer, or a copy queue).
- **One allocation per resource.** Fine for a handful of resources. A real scene will want sub-allocation (NRI can route allocations through VMA).
- **Single-mip 2D textures.** No mip chains, arrays, cube maps or 3D textures yet. Mip chains will come from the asset pipeline, which generates them offline.
- **Pipeline layouts are written by hand.** Descriptor set layouts, push constant sizes and vertex layouts must match the shader by hand. Slang's reflection API could generate or verify them when a pipeline is created.
- **Descriptor sets are never freed.** See [Resource binding](#resource-binding).
- **One vertex buffer, one color target, two blend modes.** `GraphicsPipelineSettings` covers exactly what is used so far: `BlendMode::Opaque` and `BlendMode::Alpha` (standard "over" compositing). Other blend modes, stencil and multiple render targets get settings as they are needed.
- **SDR output only.** The swap chain is 8-bit sRGB (`BT709_G22_8BIT`). Linear values above 1.0 are clipped, not tone-mapped. HDR rendering targets and tone mapping will come with the lighting work.
- **Swap chain format assumed stable.** Pipelines are created with the format reported at startup. If recreation ever produced a different format (it does not in practice for the same window and display), those pipelines would need rebuilding.

## Validation

In debug builds `GraphicsDevice` enables both NRI's validation and the graphics API's own validation. On Vulkan that is the validation layers, which come with the [Vulkan SDK](https://vulkan.lunarg.com/). On D3D12 it is the debug layer, which comes with the Windows **Graphics Tools** optional feature (Settings > System > Optional features). Without them, the device is still created, just without API validation. All validation output goes to the engine log, so a validation error shows up as a `CORE` error line.
