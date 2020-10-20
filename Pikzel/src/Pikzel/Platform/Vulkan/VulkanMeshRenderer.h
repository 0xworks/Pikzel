#pragma once

#include "VulkanDevice.h"
#include "Pikzel/Renderer/MeshRenderer.h"

namespace Pikzel {

   class VulkanMeshRenderer : public MeshRenderer {
   public:

      VulkanMeshRenderer(GraphicsContext& gc, const Model& model);
      virtual ~VulkanMeshRenderer();

      virtual void Draw(GraphicsContext& gc, const Mesh& mesh) const override;

   private:
      void CreateDescriptorPool(const Model& model);
      void DestroyDesciptorPool();

      void CreateDescriptorSets(const Model& model);
      void DestroyDescriptorSets();

   private:
      std::shared_ptr<VulkanDevice> m_Device;
      vk::DescriptorPool m_DescriptorPool;
      std::vector<vk::DescriptorSet> m_DescriptorSets;

   };

}
