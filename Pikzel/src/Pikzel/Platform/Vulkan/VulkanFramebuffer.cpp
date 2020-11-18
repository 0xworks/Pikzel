#include "VulkanFramebuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanUtility.h"

#include "imgui_impl_vulkan.h"

namespace Pikzel {

   VulkanFramebuffer::VulkanFramebuffer(std::shared_ptr<VulkanDevice> device, const FramebufferSettings& settings)
   : m_Device {device}
   , m_Settings {settings}
   {
      // TODO anti-aliasing
      m_DepthFormat = FindSupportedFormat(
         m_Device->GetVkPhysicalDevice(),
         {vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm},
         vk::ImageTiling::eOptimal,
         vk::FormatFeatureFlagBits::eDepthStencilAttachment
      );
      vk::SampleCountFlagBits sampleCount = static_cast<vk::SampleCountFlagBits>(GetMSAANumSamples());
      m_DepthImage = std::make_unique<VulkanImage>(m_Device, m_Settings.Width, m_Settings.Height, 1, sampleCount, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageCreateFlags {});
      m_DepthImage->CreateImageView(m_DepthFormat, vk::ImageAspectFlagBits::eDepth);

      m_Texture = make_unique<VulkanTexture2D>(m_Device, m_Settings.Width, m_Settings.Height, TextureFormat::BGRA8, 1);

      std::vector<vk::ImageView> attachments;

      if (sampleCount == vk::SampleCountFlagBits::e1) {
         attachments = {
            m_Texture->GetVkImageView(),
            m_DepthImage->GetVkImageView()
         };
      } else {
         m_ColorImage = std::make_unique<VulkanImage>(m_Device, m_Settings.Width, m_Settings.Height, 1, sampleCount, vk::Format::eB8G8R8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageCreateFlags {});
         m_ColorImage->CreateImageView(vk::Format::eB8G8R8A8Unorm, vk::ImageAspectFlagBits::eColor);
         attachments = {
            m_ColorImage->GetVkImageView(),
            m_DepthImage->GetVkImageView(),
            m_Texture->GetVkImageView()
         };
      }

      m_Context = std::make_unique<VulkanFramebufferGC>(m_Device, this);

      vk::FramebufferCreateInfo ci = {
         {}                                        /*flags*/,
         static_cast<VulkanFramebufferGC&>(*m_Context).GetVkRenderPass()              /*renderPass*/,
         static_cast<uint32_t>(attachments.size()) /*attachmentCount*/,
         attachments.data()                        /*pAttachments*/,
         m_Settings.Width                          /*width*/,
         m_Settings.Height                         /*height*/,
         1                                         /*layers*/
      };

      m_Framebuffer = m_Device->GetVkDevice().createFramebuffer(ci);
   }


   VulkanFramebuffer::~VulkanFramebuffer() {
      if (m_Device) {
         m_Device->GetVkDevice().waitIdle();
         if (m_DescriptorSet) {
            ImGui_ImplVulkan_DestroyTexture(reinterpret_cast<ImTextureID>(m_DescriptorSet));
            m_DescriptorSet = VK_NULL_HANDLE;
         }
         if (m_Framebuffer) {
            m_Device->GetVkDevice().destroy(m_Framebuffer);
            m_Framebuffer = nullptr;
         }
         m_Context.reset();
         m_DepthImage.reset();
         m_Texture.reset();
      }
   }


   GraphicsContext& VulkanFramebuffer::GetGraphicsContext() {
      PKZL_CORE_ASSERT(m_Context, "Accessing null graphics context!");
      return *m_Context;
   }


   uint32_t VulkanFramebuffer::GetWidth() const {
      return m_Settings.Width;
   }


   uint32_t VulkanFramebuffer::GetHeight() const {
      return m_Settings.Height;
   }


   void VulkanFramebuffer::Resize(const uint32_t width, const uint32_t height) {
      m_Context->SwapBuffers();   // The idea here is to make sure the GPU isnt still rendering to the attachments we are about to destroy
                                  // but is "swapbuffers" on the graphics context sufficient to ensure that?

      if (m_DescriptorSet) {
         ImGui_ImplVulkan_DestroyTexture(reinterpret_cast<ImTextureID>(m_DescriptorSet));
         m_DescriptorSet = VK_NULL_HANDLE;
      }
      m_Device->GetVkDevice().destroy(m_Framebuffer);
      m_Framebuffer = nullptr;

      m_Texture.reset();
      m_DepthImage.reset();

      m_Settings.Width = width;
      m_Settings.Height = height;

      m_DepthImage = std::make_unique<VulkanImage>(m_Device, m_Settings.Width, m_Settings.Height, 1, vk::SampleCountFlagBits::e1, m_DepthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageCreateFlags {});
      m_DepthImage->CreateImageView(m_DepthFormat, vk::ImageAspectFlagBits::eDepth);

      m_Texture = make_unique<VulkanTexture2D>(m_Device, m_Settings.Width, m_Settings.Height, TextureFormat::RGBA8, 1);

      std::array<vk::ImageView, 2> attachments = {
         m_Texture->GetVkImageView(),
         m_DepthImage->GetVkImageView()
      };

      vk::RenderPass renderPass = static_cast<VulkanFramebufferGC*>(m_Context.get())->GetVkRenderPass();
      vk::FramebufferCreateInfo ci = {
         {}                                        /*flags*/,
         renderPass                                /*renderPass*/,
         static_cast<uint32_t>(attachments.size()) /*attachmentCount*/,
         attachments.data()                        /*pAttachments*/,
         m_Settings.Width                          /*width*/,
         m_Settings.Height                         /*height*/,
         1                                         /*layers*/
      };

      m_Framebuffer = m_Device->GetVkDevice().createFramebuffer(ci);
   }


   uint32_t VulkanFramebuffer::GetMSAANumSamples() const {
      return m_Settings.MSAANumSamples;
   }


   const glm::vec4& VulkanFramebuffer::GetClearColor() const {
      return m_Settings.ClearColor;
   }


   const Texture2D& VulkanFramebuffer::GetColorTexture() const {
      return *m_Texture;
   }


   ImTextureID VulkanFramebuffer::GetImGuiTextureId() {
      if (!m_DescriptorSet) {
         PKZL_CORE_ASSERT(ImGui::GetIO().BackendRendererName, "Called GetImGuiTextureId, but ImGui has not been initialized.  Please call GraphicsContext::InitalizeImGui() first");
         m_DescriptorSet = reinterpret_cast<VkDescriptorSet>(ImGui_ImplVulkan_AddTexture(m_Texture->GetVkSampler(), m_Texture->GetVkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
      }
      return reinterpret_cast<ImTextureID>(m_DescriptorSet);
   }


   vk::Framebuffer VulkanFramebuffer::GetVkFramebuffer() const {
      return m_Framebuffer;
   }


   vk::Format VulkanFramebuffer::GetVkFormat() const {
      return m_Texture->GetVkFormat();
   }


   vk::Format VulkanFramebuffer::GetVkDepthFormat() const {
      return m_DepthFormat;
   }

}
