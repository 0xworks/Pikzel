#pragma once

#include "Core.h"

#include <memory>

struct GLFWwindow;

namespace Pikzel {

struct ApplicationSettings {
   const char* ApplicationName = "Pikzel::Application";
   uint32_t WindowWidth = 800;
   uint32_t WindowHeight = 600;
   bool IsResizable = true;
   bool IsFullScreen = false;
   bool IsCursorEnabled = true;
};


class Application {

public:
   Application(const ApplicationSettings& settings = {});
   virtual ~Application();

   virtual void Init();

   virtual void Run();

   virtual void OnKey(const int key, const int scancode, const int action, const int mods);
   virtual void OnCursorPos(const double xpos, const double ypos);
   virtual void OnMouseButton(const int button, const int action, const int mods);
   virtual void OnWindowResize(const int width, const int height);

protected:

   virtual void CreateWindow();
   virtual void DestroyWindow();

   virtual void Update(double deltaTime);

   virtual void Render();

protected:
   ApplicationSettings m_Settings;

   GLFWwindow* m_Window = nullptr;

   double m_AppTime = 0.0;
   double m_MouseX = 0.0;
   double m_MouseY = 0.0;

   bool m_Running = false;
   bool m_LeftMouseDown = false;

};

}


// Must be defined in client
extern std::unique_ptr<Pikzel::Application> CreateApplication(int argc, const char* argv[]);
