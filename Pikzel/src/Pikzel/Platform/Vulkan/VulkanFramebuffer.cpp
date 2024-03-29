#include "VulkanFramebuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanUtility.h"

#include "Pikzel/Renderer/RenderCore.h"

#include <backends/imgui_impl_vulkan.h>

namespace Pikzel {

   VulkanFramebuffer::VulkanFramebuffer(std::shared_ptr<VulkanDevice> device, const FramebufferSettings& settings)
   : m_Device {device}
   , m_Settings {settings}
   {
      CreateAttachments();
      m_Context = std::make_unique<VulkanFramebufferGC>(m_Device, this);
      CreateFramebuffer();
   }


   VulkanFramebuffer::~VulkanFramebuffer() {
      if (m_Device) {
         m_Device->GetVkDevice().waitIdle();
         DestroyAttachments();
         DestroyFramebuffer();
      }
   }


   GraphicsContext& VulkanFramebuffer::GetGraphicsContext() {
      PKZL_CORE_ASSERT(m_Context, "Accessing null graphics context!");
      return *m_Context;
   }


   uint32_t VulkanFramebuffer::GetWidth() const {
      return m_Settings.width;
   }


   uint32_t VulkanFramebuffer::GetHeight() const {
      return m_Settings.height;
   }


   void VulkanFramebuffer::Resize(const uint32_t width, const uint32_t height) {
      m_Context->SwapBuffers();   // The idea here is to make sure the GPU isnt still rendering to the attachments we are about to destroy
                                  // but is "swapbuffers" on the graphics context sufficient to ensure that?
      DestroyAttachments();
      DestroyFramebuffer();

      m_Settings.width = width;
      m_Settings.height = height;

      CreateAttachments();
      CreateFramebuffer();
   }


   uint32_t VulkanFramebuffer::GetMSAANumSamples() const {
      return m_Settings.msaaNumSamples;
   }


   const glm::vec4& VulkanFramebuffer::GetClearColorValue() const {
      return m_Settings.clearColorValue;
   }


   double VulkanFramebuffer::GetClearDepthValue() const {
      return m_Settings.clearDepthValue;
   }


   uint32_t VulkanFramebuffer::GetNumColorAttachments() const {
      return m_ColorTextures.size();
   }


   const Texture& VulkanFramebuffer::GetColorTexture(const int index) const {
      return *m_ColorTextures[index];
   }


   ImTextureID VulkanFramebuffer::GetImGuiColorTextureId(const int index) const {
      PKZL_CORE_ASSERT((index >= 0) && (index < m_ColorTextures.size()), "index out of range in VulkanFramebuffer::GetImColorTextureId()!");
      if (m_ColorDescriptorSets.size() < m_ColorTextures.size()) {
         m_ColorDescriptorSets.resize(m_ColorTextures.size());
      }
      if(!m_ColorDescriptorSets[index]) {
         PKZL_CORE_ASSERT(ImGui::GetIO().BackendRendererName, "Called GetImGuiTextureId, but ImGui has not been initialized.  Please call GraphicsContext::InitalizeImGui() first");
         auto texture = static_cast<VulkanTexture*>(m_ColorTextures[index].get());
         m_ColorDescriptorSets[index] = reinterpret_cast<VkDescriptorSet>(ImGui_ImplVulkan_AddTexture(texture->GetVkSampler(), texture->GetVkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
      }
      return reinterpret_cast<ImTextureID>(m_ColorDescriptorSets[index]);
   }


   bool VulkanFramebuffer::HasDepthAttachment() const {
      return m_DepthTexture != nullptr;
   }


   const Texture& VulkanFramebuffer::GetDepthTexture() const {
      return *m_DepthTexture;
   }


   ImTextureID VulkanFramebuffer::GetImGuiDepthTextureId() const {
      if (!m_DepthDescriptorSet) {
         PKZL_CORE_ASSERT(ImGui::GetIO().BackendRendererName, "Called GetImGuiTextureId, but ImGui has not been initialized.  Please call GraphicsContext::InitalizeImGui() first");
         auto texture = static_cast<VulkanTexture*>(m_DepthTexture.get());
         m_DepthDescriptorSet = reinterpret_cast<VkDescriptorSet>(ImGui_ImplVulkan_AddTexture(texture->GetVkSampler(), texture->GetVkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
      }
      return reinterpret_cast<ImTextureID>(m_DepthDescriptorSet);
   }


   vk::Framebuffer VulkanFramebuffer::GetVkFramebuffer() const {
      return m_Framebuffer;
   }


   std::vector<vk::AttachmentDescription2>& VulkanFramebuffer::GetVkAttachments() {
      return m_Attachments;
   }


   void VulkanFramebuffer::CreateAttachments() {
      bool isMultiSampled = m_Settings.msaaNumSamples > 1;
      vk::SampleCountFlagBits sampleCount = static_cast<vk::SampleCountFlagBits>(GetMSAANumSamples());

      m_ImageViews.clear();
      m_ImageViews.reserve(m_Settings.attachments.size() * (isMultiSampled? 2 : 1));
      uint32_t numColorAttachments = 0;
      uint32_t numDepthAttachments = 0;
      m_LayerCount = 1;
      for (const auto attachment : m_Settings.attachments) {
         switch (attachment.attachmentType) {
            case AttachmentType::Color: {
               PKZL_CORE_ASSERT(numColorAttachments < 4, "Framebuffer can have a maximum of four color attachments!");
               if (isMultiSampled) {
                  vk::ImageViewType type;
                  switch (attachment.textureType) {
                     case TextureType::Texture2D:
                        type = vk::ImageViewType::e2D;
                        break;
                     case TextureType::Texture2DArray:
                        type = vk::ImageViewType::e2DArray;
                        break;
                     case TextureType::TextureCube:
                     case TextureType::TextureCubeArray:
                        type = vk::ImageViewType::eCube;
                        break;
                     default:
                        PKZL_CORE_ASSERT(false, "unknown attachment texture type!");
                  }
                  m_MSAAColorImages.emplace_back(std::make_unique<VulkanImage>(m_Device, type, m_Settings.width, m_Settings.height, m_Settings.layers, 1, sampleCount, TextureFormatToVkFormat(attachment.format), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vma::MemoryUsage::eGpuOnly));
                  m_MSAAColorImages.back()->CreateImageViews(TextureFormatToVkFormat(attachment.format), vk::ImageAspectFlagBits::eColor);
                  m_ImageViews.push_back(m_MSAAColorImages.back()->GetVkImageView());
                  m_Attachments.push_back({
                     {}                                               /*flags*/,
                     TextureFormatToVkFormat(attachment.format)       /*format*/,
                     sampleCount                                      /*samples*/,
                     vk::AttachmentLoadOp::eClear                     /*loadOp*/,
                     vk::AttachmentStoreOp::eDontCare                 /*storeOp*/,
                     vk::AttachmentLoadOp::eDontCare                  /*stencilLoadOp*/,
                     vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
                     vk::ImageLayout::eUndefined                      /*initialLayout*/,
                     vk::ImageLayout::eColorAttachmentOptimal         /*finalLayout*/
                  });
               }
               m_ColorTextures.emplace_back(RenderCore::CreateTexture({
                  .textureType = attachment.textureType,
                  .width = m_Settings.width,
                  .height = m_Settings.height,
                  .layers = m_Settings.layers,
                  .format = attachment.format,
                  .wrapU = TextureWrap::ClampToEdge,
                  .wrapV = TextureWrap::ClampToEdge,
                  .mipLevels = 1
               }));
               m_ImageViews.push_back(static_cast<VulkanTexture*>(m_ColorTextures.back().get())->GetVkImageView());
               m_LayerCount = m_ColorTextures.back()->GetLayers();
               m_Attachments.push_back({
                  {}                                                                              /*flags*/,
                  TextureFormatToVkFormat(attachment.format)                                      /*format*/,
                  vk::SampleCountFlagBits::e1                                                     /*samples*/,
                  isMultiSampled? vk::AttachmentLoadOp::eDontCare : vk::AttachmentLoadOp::eClear  /*loadOp*/,
                  vk::AttachmentStoreOp::eStore                                                   /*storeOp*/,
                  vk::AttachmentLoadOp::eDontCare                                                 /*stencilLoadOp*/,
                  vk::AttachmentStoreOp::eDontCare                                                /*stencilStoreOp*/,
                  vk::ImageLayout::eUndefined                                                     /*initialLayout*/,
                  vk::ImageLayout::eShaderReadOnlyOptimal                                         /*finalLayout*/
               });
               ++numColorAttachments;
               break;
            }
            case AttachmentType::Depth:
            case AttachmentType::DepthStencil: {
               PKZL_CORE_ASSERT(numDepthAttachments < 1, "Framebuffer can only have one depth attachment!");
               if (isMultiSampled) {
                  vk::ImageViewType type;
                  switch (attachment.textureType) {
                     case TextureType::Texture2D:
                        type = vk::ImageViewType::e2D;
                        break;
                     case TextureType::Texture2DArray:
                        type = vk::ImageViewType::e2DArray;
                        break;
                     case TextureType::TextureCube:
                     case TextureType::TextureCubeArray:
                        type = vk::ImageViewType::eCube;
                        break;
                     default:
                        PKZL_CORE_ASSERT(false, "unknown attachment texture type!");
                  }
                  m_MSAADepthImage = std::make_unique<VulkanImage>(m_Device, type, m_Settings.width, m_Settings.height, m_Settings.layers, 1, sampleCount, TextureFormatToVkFormat(attachment.format), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment, vma::MemoryUsage::eGpuOnly);
                  m_MSAADepthImage->CreateImageViews(TextureFormatToVkFormat(attachment.format), vk::ImageAspectFlagBits::eDepth);
                  m_ImageViews.push_back(m_MSAADepthImage->GetVkImageView());
                  m_Attachments.push_back({
                     {}                                               /*flags*/,
                     TextureFormatToVkFormat(attachment.format)       /*format*/,
                     sampleCount                                      /*samples*/,
                     vk::AttachmentLoadOp::eClear                     /*loadOp*/,
                     vk::AttachmentStoreOp::eDontCare                 /*storeOp*/,
                     vk::AttachmentLoadOp::eClear                     /*stencilLoadOp*/,
                     vk::AttachmentStoreOp::eDontCare                 /*stencilStoreOp*/,
                     vk::ImageLayout::eUndefined                      /*initialLayout*/,
                     vk::ImageLayout::eDepthStencilAttachmentOptimal  /*finalLayout*/
                  });
               }
               m_DepthTexture = RenderCore::CreateTexture({
                  .textureType = attachment.textureType,
                  .width = m_Settings.width,
                  .height = m_Settings.height,
                  .layers = m_Settings.layers,
                  .format = attachment.format,
                  .wrapU = TextureWrap::ClampToBorder,
                  .wrapV = TextureWrap::ClampToBorder,
                  .mipLevels = 1
               });
               m_ImageViews.push_back(static_cast<VulkanTexture*>(m_DepthTexture.get())->GetVkImageView());
               m_LayerCount = m_DepthTexture->GetLayers();
               m_Attachments.push_back({
                  {}                                                                              /*flags*/,
                  TextureFormatToVkFormat(attachment.format)                                      /*format*/,
                  vk::SampleCountFlagBits::e1                                                     /*samples*/,
                  isMultiSampled ? vk::AttachmentLoadOp::eDontCare : vk::AttachmentLoadOp::eClear  /*loadOp*/,
                  vk::AttachmentStoreOp::eStore                                                   /*storeOp*/,
                  isMultiSampled ? vk::AttachmentLoadOp::eDontCare : vk::AttachmentLoadOp::eClear  /*stencilLoadOp*/,
                  vk::AttachmentStoreOp::eStore                                                   /*stencilStoreOp*/,
                  vk::ImageLayout::eUndefined                                                     /*initialLayout*/,
                  vk::ImageLayout::eDepthStencilAttachmentOptimal                                 /*finalLayout*/
               });
               ++numDepthAttachments;
               break;
            }
            default: {
               PKZL_CORE_ASSERT(false, "Unknown attachment type!");
            }
         }
      }
   }


   void VulkanFramebuffer::DestroyAttachments() {
      for (auto& descriptorSet : m_ColorDescriptorSets) {
         ImGui_ImplVulkan_RemoveTexture(descriptorSet);
      }
      m_ColorDescriptorSets.clear();
      if (m_DepthDescriptorSet) {
         ImGui_ImplVulkan_RemoveTexture(m_DepthDescriptorSet);
         m_DepthDescriptorSet = VK_NULL_HANDLE;
      }
      m_Context.reset();
      m_MSAADepthImage.reset();
      m_DepthTexture.reset();
      m_MSAAColorImages.clear();
      m_ColorTextures.clear();
   }


   void VulkanFramebuffer::CreateFramebuffer() {
      vk::FramebufferCreateInfo ci = {
         {}                                                                                     /*flags*/,
         static_cast<VulkanFramebufferGC&>(*m_Context).GetVkRenderPass(BeginFrameOp::ClearAll)  /*renderPass*/,
         static_cast<uint32_t>(m_ImageViews.size())                                             /*attachmentCount*/,
         m_ImageViews.data()                                                                    /*pAttachments*/,
         m_Settings.width                                                                       /*width*/,
         m_Settings.height                                                                      /*height*/,
         m_LayerCount                                                                           /*layers*/
      };
      m_Framebuffer = m_Device->GetVkDevice().createFramebuffer(ci);
   }


   void VulkanFramebuffer::DestroyFramebuffer() {
      if (m_Framebuffer) {
         m_Device->GetVkDevice().destroy(m_Framebuffer);
         m_Framebuffer = nullptr;
      }
   }
}
