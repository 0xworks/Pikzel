# Pikzel

A modular, high-performance game engine built from scratch in modern C++.

## Overview

Pikzel is a general-purpose game engine designed with a particular emphasis on real-time rendering, animation, inverse kinematics, physics and AI.

The project aims to provide a clean, modular and extensible foundation for game development, combining established third-party technologies with custom-built engine systems.

Rather than attempting to replicate the feature set of large commercial engines, the focus is on a well-designed architecture, performance, flexibility and high-quality core systems.

**This project is currently in early development.** Architecture, technology choices and implementation details are subject to change.

## Design Goals

- **Modular architecture** - independently developed and usable engine subsystems with clearly defined interfaces.
- **Performance** - efficient use of modern CPU and GPU capabilities, with multithreading where appropriate.
- **Modern rendering** - cross-platform graphics abstraction with support for contemporary rendering techniques.
- **Advanced animation** - first-class support for skeletal animation, blending, procedural animation and animation graphs.
- **Inverse kinematics** - flexible constraint-based IK and skeletal manipulation.
- **Physics** - integrated real-time rigid-body physics and collision detection.
- **AI** - extensible character behaviour, decision-making and navigation systems.
- **Extensible scripting** - C# scripting integrated with the native C++ runtime.
- **Integrated tooling** - a dedicated editor for scene management, asset creation and debugging.

## Learning and Code Quality

This project is intended not only to be a functional game engine, but also a useful resource for developers interested in understanding modern C++ game engine design and development.

The goal is to strike a balance between technical depth and approachability. The engine should demonstrate real-world techniques and sound engineering practices without becoming unnecessarily complicated or relying on layers of abstraction that obscure how things work.

Guiding principles include:

- **Readable, understandable code** - favour clarity and straightforward implementations over cleverness or unnecessary complexity.
- **Practical engineering** - implement genuine engine systems rather than overly simplified demonstrations.
- **Transparent design decisions** - document significant architectural choices, including the reasoning behind them and relevant trade-offs.
- **Focused abstractions** - introduce abstractions where they solve real problems, not simply to anticipate hypothetical future requirements.
- **Independent subsystems** - keep major engine systems sufficiently self-contained that they can be studied and understood individually.
- **Modern C++ practices** - demonstrate effective use of the language and standard library, including ownership, resource management, concurrency and performance considerations.

The intention is to make the codebase accessible to developers who already have a reasonable understanding of C++ and want to deepen their knowledge of game engine development. It is not intended as a beginner programming tutorial, nor as an exercise in maximising architectural sophistication.

## AI-Assisted Development

AI tools are used throughout the development of this project, including code generation, implementation assistance, research, debugging and exploring alternative approaches.

The architectural direction, technology choices and overall design decisions remain my responsibility as the project author. AI-generated code is reviewed, evaluated and tested before being incorporated into the codebase.

AI is treated as a development tool rather than an autonomous engineer. Its suggestions are not automatically accepted, and particular care is taken to ensure that implementations are technically sound, maintainable and consistent with the project's architectural principles.

The use of AI is also an opportunity to explore how these tools can be applied effectively to a substantial modern C++ software project, including their strengths, limitations and the engineering judgement still required to use them well.

## Technology Stack

The project is built around the following technologies.

| Technology | Purpose |
|---|---|
| C++23 | Primary implementation language |
| CMake | Build system |
| NVIDIA NRI | Graphics API abstraction |
| Slang | Shader language and compiler |
| EnTT | Entity Component System |
| RTM | Real-time mathematics |
| Box3D | Physics simulation |
| GLFW | Windowing and input |
| Dear ImGui | Editor interface |
| .NET / C# | Managed scripting |
| miniaudio | Audio backend |
| fastgltf | glTF asset importing |
| meshoptimizer | Mesh optimisation |
| KTX-Software | Texture processing |

Additional libraries and technologies may be introduced as development progresses.

## Architecture

The engine is organised around three major components.

### Runtime

The runtime contains the systems required to execute a game independently of the editor.

Planned subsystems include:

- Scene and entity management
- Rendering
- Physics
- Animation and IK
- AI and character systems
- Scripting
- Audio
- Resource management

### Editor

The editor provides development and authoring tools built using Dear ImGui.

Planned functionality includes:

- Scene hierarchy and entity inspection
- Component editing
- Scene viewport and transform gizmos
- Asset browsing and management
- Animation graph authoring
- Physics and animation debugging
- Performance profiling

### Asset Compiler

A separate asset-processing pipeline will convert source assets into runtime-ready resources.

This will support model importing, texture processing, mesh optimisation, animation processing and compiled asset generation.

## Animation and IK

Animation is a major focus of the engine.

The intention is to implement a custom animation system supporting:

- Skeletal animation
- Animation clips and blending
- Animation state machines
- Layered animation and masks
- Animation graphs
- Procedural animation
- Inverse kinematics
- Animation retargeting
- Physics-driven animation
- Motion matching

A custom node graph framework will provide the foundation for visual animation authoring, without relying on non-standard Dear ImGui extensions.

## Scripting

The engine will support C# scripting through an embedded .NET runtime.

The native/managed integration will be implemented specifically for the engine, with an emphasis on explicit ownership, stable entity handles and a well-defined scripting API.

## Development Philosophy

A few principles guide development:

- Prefer simple, maintainable solutions over unnecessary abstraction.
- Use the C++ standard library wherever appropriate.
- Avoid unnecessary third-party dependencies.
- Keep engine subsystems independent.
- Separate editor functionality from runtime functionality.
- Profile before optimising.
- Build incrementally, keeping the engine compilable and testable throughout development.


## Building

Requirements:

- CMake 3.28 or later
- Ninja
- A C++23 compiler (currently developed with MSVC from Visual Studio 2026)

Third-party dependencies are fetched automatically by CMake during the first configure (see `cmake/Dependencies.cmake`).

On Windows, run these from a Visual Studio x64 developer prompt (or open the folder in Visual Studio or VS Code, which pick up the presets):

```bash
cmake --preset windows-x64-debug
```

```bash
cmake --build --preset windows-x64-debug
```

```bash
build/windows-x64-debug/bin/HelloLog
```

Linux presets (`linux-debug`, `linux-release`) are also defined.

### Repository Layout

| Path | Contents |
|---|---|
| `src/Pikzel/` | Engine runtime library, one folder per subsystem (`Core/`, ...) |
| `examples/` | Small programs, each demonstrating one part of the engine |
| `cmake/` | Shared CMake modules: compiler options and third-party dependencies |


## Project Status

<em>Early development / Initial architecture</em>

The project is currently at the foundation and architecture stage.

Planned development milestones:
- [ ] Core infrastructure and platform layer
- [ ] Rendering foundation
- [ ] Scene management and ECS integration
- [ ] Editor foundation
- [ ] Asset pipeline
- [ ] Physics integration
- [ ] Animation and IK
- [ ] Animation graph editor
- [ ] C# scripting
- [ ] AI, audio and advanced rendering features


## License

This project is licensed under the MIT License.

You are free to use, modify, distribute and incorporate the engine into personal or commercial projects, subject to the terms of the MIT License.

See the LICENSE file for the full licence text.

Third-party dependencies retain their respective licences and copyright notices.