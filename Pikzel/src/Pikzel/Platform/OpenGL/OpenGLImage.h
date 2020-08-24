#pragma once

#include "Pikzel/Renderer/Image.h"

namespace Pikzel {

   class OpenGLImage : public Image {
   public:

      virtual void* GetRendererId() const override {
         throw std::logic_error("The method or operation is not implemented.");
      }

   };

}