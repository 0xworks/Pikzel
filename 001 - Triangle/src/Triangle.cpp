#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/Window.h"
#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/WindowEvents.h"
#include "Pikzel/Renderer/RenderCore.h"

// Render a triangle in a window

class Triangle final : public Pikzel::Application {
public:
   Triangle() {
      PKZL_PROFILE_FUNCTION();
      m_Window = Pikzel::Window::Create({APP_DESCRIPTION});
      Pikzel::EventDispatcher::Connect<Pikzel::WindowCloseEvent, &Triangle::OnWindowClose>(*this);
      Pikzel::RenderCore::SetClearColor({1.0f, 0.0f, 0.0f, 1.0f});
   }


   virtual void Update(Pikzel::DeltaTime deltaTime) override {
      ;
   }


   virtual void Render() override {
      PKZL_PROFILE_FUNCTION();

      m_Window->BeginFrame();

      Pikzel::RenderCore::Clear();

      //
      // TODO:  render triangle here...
      //

      m_Window->EndFrame();

   }


private:
   void OnWindowClose(const Pikzel::WindowCloseEvent& event) {
      if (event.Sender == m_Window->GetNativeWindow()) {
         Exit();
      }
   }


private:
   std::unique_ptr<Pikzel::Window> m_Window;

};


std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<Triangle>();
}
