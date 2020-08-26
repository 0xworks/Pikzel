#include "pch.h"
#include "RenderCore.h"


namespace Pikzel {

   // TODO: remove:
   extern std::unique_ptr<IRenderCore> Create();

   RenderCore::API RenderCore::s_API = RenderCore::API::None;

   std::unique_ptr<IRenderCore> RenderCore::s_RenderCore;


   void RenderCore::SetAPI(API api) {
      PKZL_CORE_ASSERT(s_API == API::None, "Rendercore API is already set!");
      if (s_API == API::None) {
         s_API = api;
      }
   }


   RenderCore::API RenderCore::GetAPI() {
      return s_API;
   }


   void RenderCore::Init() {
      //
      // this is not a switch on s_API with call to corresponding XXXRenderCore constructor
      // because this file should not have to know about XXXRenderCore
      // All it needs to know is that _somewhere_ there is a function to call to get back
      // some object that implements IRenderCore
      s_RenderCore = Create();
   }


   void RenderCore::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
      s_RenderCore->SetViewport(x, y, width, height);
   }


   void RenderCore::SetClearColor(const glm::vec4& color) {
      s_RenderCore->SetClearColor(color);
   }


   void RenderCore::Clear() {
      s_RenderCore->Clear();
   }


   std::unique_ptr<GraphicsContext> RenderCore::CreateGraphicsContext(Window& window) {
      return s_RenderCore->CreateGraphicsContext(window);
   }

}
