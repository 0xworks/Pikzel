#pragma once

#include <cstdint>
#include <string>
#include <variant>

struct GLFWwindow;

namespace Pikzel {

struct WindowSettings {
   std::string title = "Pikzel";
   uint32_t width = 1280;
   uint32_t height = 720;
   bool isResizable = true;
};

// Operating-system handles that identify a window to a graphics API.
//
// The renderer needs these to create a swap chain, but the platform layer should not
// depend on the renderer (or the other way round on GLFW), so the window hands out plain
// OS handles and the renderer converts them into whatever its graphics API expects.
struct Win32WindowHandle {
   void* hwnd = nullptr; // HWND
};

struct X11WindowHandle {
   void* display = nullptr; // Display*
   uint64_t window = 0;     // Window
};

struct WaylandWindowHandle {
   void* display = nullptr; // wl_display*
   void* surface = nullptr; // wl_surface*
};

using NativeWindowHandle = std::variant<Win32WindowHandle, X11WindowHandle, WaylandWindowHandle>;

struct FramebufferSize {
   uint32_t width = 0;
   uint32_t height = 0;
};

// A desktop window, implemented with GLFW.
//
// The window creates no graphics context of its own (GLFW_NO_API): rendering into it is
// the job of a swap chain created by the renderer.
//
// GLFW is a process-wide library that must be initialised before the first window is
// created and terminated after the last is destroyed. Window does this itself by counting
// live windows, so applications never call GLFW directly. Like GLFW itself, windows must
// be created, used and destroyed on the main thread.
class Window {
public:
   explicit Window(const WindowSettings& settings = {});
   ~Window();

   Window(const Window&) = delete;
   Window& operator=(const Window&) = delete;
   Window(Window&&) = delete;
   Window& operator=(Window&&) = delete;

   // Processes pending events for all windows. Call once per frame.
   static void PollEvents();

   // Blocks until at least one event is available, then processes it. Useful while a
   // window is minimised, to avoid spinning when there is nothing to draw.
   static void WaitEvents();

   bool ShouldClose() const;

   // The size of the window's drawable area in pixels. On high-DPI displays this can
   // differ from the window size in screen coordinates. Zero while the window is minimised.
   FramebufferSize GetFramebufferSize() const;

   bool IsMinimized() const;

   NativeWindowHandle GetNativeHandle() const;

private:
   GLFWwindow* m_Window = nullptr;
};

}
