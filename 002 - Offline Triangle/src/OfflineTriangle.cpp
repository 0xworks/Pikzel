#include "Pikzel/Core/Application.h"
#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/Image.h"
#include "Pikzel/Renderer/Renderer.h"



// NOT FINISHED 



// This is a small test to render a triangle.
// The render is offline - there is no window to visualize, or interact with, the image.
//

class OfflineTriangle final : public Pikzel::Application {
public:
   OfflineTriangle() {
      PKZL_PROFILE_FUNCTION();
      m_Image = Pikzel::Renderer::CreateImage();
      m_ImageGC = Pikzel::Renderer::CreateGraphicsContext(*m_Image);
   }


   virtual void Render() override {
       PKZL_PROFILE_FUNCTION();

      m_ImageGC->BeginFrame();
      //
      // TODO:  render triangle here...
      //
      m_ImageGC->EndFrame();    // this submits the frame... but it the GPU hasn't necessarily rendered it yet
      m_ImageGC->SwapBuffers(); // this blocks until render is done

      // TODO copy image from device to host here
      // TODO and save it out somewhere...
   }

private:
   std::unique_ptr<Pikzel::Image> m_Image;
   std::unique_ptr<Pikzel::GraphicsContext> m_ImageGC;

};



std::unique_ptr<Pikzel::Application> Pikzel::CreateApplication(int argc, const char* argv[]) {
   PKZL_PROFILE_FUNCTION();
   PKZL_LOG_INFO(APP_DESCRIPTION);
   PKZL_LOG_INFO("Linked against {0} {1}", PKZL_DESCRIPTION, PKZL_VERSION);
#ifdef PKZL_DEBUG
   PKZL_LOG_INFO("DEBUG build");
#endif

   return std::make_unique<OfflineTriangle>();
}
