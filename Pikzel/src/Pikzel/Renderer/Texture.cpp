#include "Texture.h"
#include "Pikzel/Core/Utility.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define DDSKTX_IMPLEMENT
#include "dds-ktx.h"

#include <filesystem>
namespace Pikzel {

   TextureLoader::TextureLoader(const std::filesystem::path& path) {
      m_FileData = ReadFile<uint8_t>(path);
      if (!TrySTBI()) {
         if (!TryDDSKTX()) {
            PKZL_CORE_ASSERT(false, "'{0}': Image format not supported!", path.string());
         }
         Flip();
      }
   }


   TextureLoader::TextureLoader(void* pData, uint32_t size) {
      m_FileData = std::vector<uint8_t>(static_cast<uint8_t*>(pData), static_cast<uint8_t*>(pData) + size);
      if (!TrySTBI()) {
         if (!TryDDSKTX()) {
            PKZL_CORE_ASSERT(false, "<embedded texture>: Image format not supported!");
         }
         Flip();
      }
   }
   
   TextureLoader::~TextureLoader() {
      if (m_Data) {
         if (m_IsDDSKTX) {
            delete static_cast<ddsktx_texture_info*>(m_Data);
         } else {
            stbi_image_free(m_Data);
         }
      }
   }


   bool TextureLoader::IsLoaded() const {
      return m_Data != nullptr;
   }


   std::pair<const void*, const uint32_t> TextureLoader::GetData(const uint32_t layer, const uint32_t slice, const uint32_t mipLevel) const {
      if (m_IsDDSKTX) {
         auto info = static_cast<ddsktx_texture_info*>(m_Data);
         ddsktx_sub_data sub;
         ddsktx_get_sub(info, &sub, m_FileData.data(), m_FileData.size(), layer, slice, mipLevel);
         return { sub.buff, sub.size_bytes };
      } else {
         PKZL_CORE_ASSERT(layer == 0, "TextureLoader::GetData() expects layer parameter to be zero for non-dds/ktx file formats!");
         PKZL_CORE_ASSERT(slice == 0, "TextureLoader::GetData() expects slice parameter to be zero for non-dds/ktx file formats!");
         PKZL_CORE_ASSERT(mipLevel == 0, "TextureLoader::GetData() expects mipLevel parameter to be zero for non-dds/ktx file formats!");
         return { m_Data, m_Width * m_Height * Texture::BPP(m_Format) };
      }
   }


   uint32_t TextureLoader::GetWidth() const {
      return m_Width;
   }


   uint32_t TextureLoader::GetHeight() const {
      return m_Height;
   }


   uint32_t TextureLoader::GetDepth() const {
      return m_Depth;
   }


   uint32_t TextureLoader::GetLayers() const {
      return m_Layers;
   }


   uint32_t TextureLoader::GetMIPLevels() const {
      return m_MIPLevels;
   }


   TextureFormat TextureLoader::GetFormat() const {
      return m_Format;
   }


   bool TextureLoader::IsCompressed() const {
      return m_IsCompressed;
   }


   bool TextureLoader::IsCubeMap() const {
      return m_IsCubeMap;
   }


   bool TextureLoader::TrySTBI() {
      int iWidth;
      int iHeight;
      int channels;
      stbi_set_flip_vertically_on_load(1);
      bool isHDR = stbi_is_hdr_from_memory(m_FileData.data(), static_cast<int>(m_FileData.size()));
      if (isHDR) {
         m_Data = reinterpret_cast<uint8_t*>(stbi_loadf_from_memory(m_FileData.data(), static_cast<int>(m_FileData.size()), &iWidth, &iHeight, &channels, STBI_rgb_alpha));
         if ((channels == 1) || (channels == 2)) {
            // if it was a 1 or 2 channel texture, then we should reload it with the actual number of channels
            // instead of forcing 4
            // (unfortunately no way to determine channels before loading the whole thing!)
            m_Data = reinterpret_cast<stbi_uc*>(stbi_loadf_from_memory(m_FileData.data(), static_cast<int>(m_FileData.size()), &iWidth, &iHeight, &channels, 0));
         }
      } else {
         m_Data = stbi_load_from_memory(m_FileData.data(), static_cast<int>(m_FileData.size()), &iWidth, &iHeight, &channels, STBI_rgb_alpha);
         if ((channels == 1) || (channels == 2)) {
            // if it was a 1 or 2 channel texture, then we should reload it with the actual number of channels
            // instead of forcing 4
            // (unfortunately no way to determine channels before loading the whole thing!)
            m_Data = stbi_load_from_memory(m_FileData.data(), static_cast<int>(m_FileData.size()), &iWidth, &iHeight, &channels, 0);
         }
      }

      if (m_Data) {
         m_IsDDSKTX = false;
         m_IsCompressed = false;
         m_Depth = 1;
         m_Layers = 1;
         m_MIPLevels = 1;
         m_Width = static_cast<uint32_t>(iWidth);
         m_Height = static_cast<uint32_t>(iHeight);
         if ((channels == 3) || (channels == 4)) {
            // we forced in an alpha channel in the load above (no 24-bit textures here)
            m_Format = isHDR ? TextureFormat::RGBA32F : TextureFormat::RGBA8;
         } else if (channels == 1) {
            m_Format = isHDR ? TextureFormat::R32F : TextureFormat::R8;
         } else {
            return false;
         }
      }

      return m_Data ? true : false;
   }


   TextureFormat DDSKTXFormatToTextureFormat(ddsktx_format format, bool isSRGB) {
      switch (format) {
         case DDSKTX_FORMAT_BC1:       return isSRGB ? TextureFormat::DXT1SRGBA : TextureFormat::DXT1RGBA;
         case DDSKTX_FORMAT_BC2:       return isSRGB ? TextureFormat::DXT3SRGBA : TextureFormat::DXT3RGBA;
         case DDSKTX_FORMAT_BC3:       return isSRGB ? TextureFormat::DXT5SRGBA : TextureFormat::DXT5RGBA;
         case DDSKTX_FORMAT_BC4:       return TextureFormat::RGTC1R;  /* signed variant? */
         case DDSKTX_FORMAT_BC5:       return TextureFormat::RGTC2RG; /* signed variant? */
         case DDSKTX_FORMAT_BC6H:      break;  /* BC6H */
         case DDSKTX_FORMAT_BC7:       break;  /* BC7 */
         case DDSKTX_FORMAT_ETC1:      break;  /* ETC1 RGB8 */
         case DDSKTX_FORMAT_ETC2:      break;  /* ETC2 RGB8 */
         case DDSKTX_FORMAT_ETC2A:     break;  /* ETC2 RGBA8 */
         case DDSKTX_FORMAT_ETC2A1:    break;  /* ETC2 RGBA8A1 */
         case DDSKTX_FORMAT_PTC12:     break;  /* PVRTC1 RGB 2bpp */
         case DDSKTX_FORMAT_PTC14:     break;  /* PVRTC1 RGB 4bpp */
         case DDSKTX_FORMAT_PTC12A:    break;  /* PVRTC1 RGBA 2bpp */
         case DDSKTX_FORMAT_PTC14A:    break;  /* PVRTC1 RGBA 4bpp */
         case DDSKTX_FORMAT_PTC22:     break;  /* PVRTC2 RGBA 2bpp */
         case DDSKTX_FORMAT_PTC24:     break;  /* PVRTC2 RGBA 4bpp */
         case DDSKTX_FORMAT_ATC:       break;  /* ATC RGB 4BPP */
         case DDSKTX_FORMAT_ATCE:      break;  /* ATCE RGBA 8 BPP explicit alpha */
         case DDSKTX_FORMAT_ATCI:      break;  /* ATCI RGBA 8 BPP interpolated alpha */
         case DDSKTX_FORMAT_ASTC4x4:   break;  /* ASTC 4x4 8.0 BPP */
         case DDSKTX_FORMAT_ASTC5x5:   break;  /* ASTC 5x5 5.12 BPP */
         case DDSKTX_FORMAT_ASTC6x6:   break;  /* ASTC 6x6 3.56 BPP */
         case DDSKTX_FORMAT_ASTC8x5:   break;  /* ASTC 8x5 3.20 BPP */
         case DDSKTX_FORMAT_ASTC8x6:   break;  /* ASTC 8x6 2.67 BPP */
         case DDSKTX_FORMAT_ASTC10x5:  break;  /* ASTC 10x5 2.56 BPP */
         case DDSKTX_FORMAT_A8:        break;
         case DDSKTX_FORMAT_R8:        return TextureFormat::R8;
         case DDSKTX_FORMAT_RGBA8:     return isSRGB ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
         case DDSKTX_FORMAT_RGBA8S:    break;
         case DDSKTX_FORMAT_RG16:      return TextureFormat::RG16F;
         case DDSKTX_FORMAT_RGB8:      return isSRGB ? TextureFormat::SRGB8 : TextureFormat::RGB8;
         case DDSKTX_FORMAT_R16:       break;
         case DDSKTX_FORMAT_R32F:      return TextureFormat::R32F;
         case DDSKTX_FORMAT_R16F:      break;
         case DDSKTX_FORMAT_RG16F:     return TextureFormat::RG16F;
         case DDSKTX_FORMAT_RG16S:     break;
         case DDSKTX_FORMAT_RGBA16F:   return TextureFormat::RGBA16F;
         case DDSKTX_FORMAT_RGBA16:    return TextureFormat::RGBA16F;
         case DDSKTX_FORMAT_BGRA8:     return TextureFormat::BGRA8;
         case DDSKTX_FORMAT_RGB10A2:   break;
         case DDSKTX_FORMAT_RG11B10F:  break;
         case DDSKTX_FORMAT_RG8:       break;
         case DDSKTX_FORMAT_RG8S:      break;
         default:                      break;
      }
      PKZL_CORE_ASSERT(false, "Unsupported TextureFormat!");
      return TextureFormat::Undefined;
   }


   bool TextureLoader::TryDDSKTX() {
      m_Data = new ddsktx_texture_info;
      ddsktx_texture_info* info = static_cast<ddsktx_texture_info*>(m_Data);
      bool isParsed = false;
      if (ddsktx_parse(info, m_FileData.data(), m_FileData.size())) {
         isParsed = true;
         m_IsDDSKTX = true;
         m_Width = info->width;
         m_Height = info->height;
         m_Depth = info->depth;
         m_Layers = info->num_layers;
         m_MIPLevels = info->num_mips;
         m_Format = DDSKTXFormatToTextureFormat(info->format, info->flags & DDSKTX_TEXTURE_FLAG_SRGB);
         m_IsCubeMap = info->flags & DDSKTX_TEXTURE_FLAG_CUBEMAP;
         m_IsCompressed = ddsktx_format_compressed(info->format);
      } else {
         delete static_cast<ddsktx_texture_info*>(m_Data);
         m_Data = nullptr;
      }
      return isParsed;
   }


   void TextureLoader::Flip() {
      for (uint32_t layer = 0; layer < m_Layers; ++layer) {
         for (uint32_t slice = 0; slice < m_Depth ; ++slice) {
            for (uint32_t level = 0; level < m_MIPLevels; ++level) {
               Flip(layer, slice, level);
            }
         }
      }
   }


   static void FlipNonCompressed(const ddsktx_sub_data& sub) {
      for (uint32_t y = 0; y < sub.height / 2; ++y) {
         auto line0 = (uint8_t*)sub.buff + y * sub.row_pitch_bytes;
         auto line1 = (uint8_t*)sub.buff + (sub.height - y - 1) * sub.row_pitch_bytes;
         for (uint32_t i = 0; i < sub.row_pitch_bytes; ++i) {
            std::swap(*line0, *line1);
            ++line0;
            ++line1;
         }
      }
   }


   static void FlipBC1(const ddsktx_sub_data& sub) {
      struct BC1Block {
         uint16_t m_color0;
         uint16_t m_color1;
         uint8_t m_row0;
         uint8_t m_row1;
         uint8_t m_row2;
         uint8_t m_row3;
      };

      uint32_t numXBlocks = (sub.width + 3) / 4;
      uint32_t numYBlocks = (sub.height + 3) / 4;
      if (sub.height == 1) {
      } else if (sub.height == 2) {
         auto blocks = (BC1Block*)sub.buff;
         for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            std::swap(block->m_row0, block->m_row1);
            std::swap(block->m_row2, block->m_row3);
         }
      } else {
         for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC1Block*)((uint8_t*)sub.buff + y * sub.row_pitch_bytes);
            auto blocks1 = (BC1Block*)((uint8_t*)sub.buff + (numYBlocks - y - 1) * sub.row_pitch_bytes);
            for (uint32_t x = 0; x < numXBlocks; x++) {
               auto block0 = blocks0 + x;
               auto block1 = blocks1 + x;
               if (blocks0 != blocks1) {
                  std::swap(block0->m_color0, block1->m_color0);
                  std::swap(block0->m_color1, block1->m_color1);
                  std::swap(block0->m_row0, block1->m_row3);
                  std::swap(block0->m_row1, block1->m_row2);
                  std::swap(block0->m_row2, block1->m_row1);
                  std::swap(block0->m_row3, block1->m_row0);
               } else {
                  std::swap(block0->m_row0, block0->m_row3);
                  std::swap(block0->m_row1, block0->m_row2);
               }
            }
         }
      }
   }


   static void FlipBC2(const ddsktx_sub_data& sub) {
      struct BC2Block {
         uint16_t m_alphaRow0;
         uint16_t m_alphaRow1;
         uint16_t m_alphaRow2;
         uint16_t m_alphaRow3;
         uint16_t m_color0;
         uint16_t m_color1;
         uint8_t m_row0;
         uint8_t m_row1;
         uint8_t m_row2;
         uint8_t m_row3;
      };

      uint32_t numXBlocks = (sub.width + 3) / 4;
      uint32_t numYBlocks = (sub.height + 3) / 4;
      if (sub.height == 1) {
      } else if (sub.height == 2) {
         auto blocks = (BC2Block*)sub.buff;
         for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            std::swap(block->m_alphaRow0, block->m_alphaRow1);
            std::swap(block->m_alphaRow2, block->m_alphaRow3);
            std::swap(block->m_row0, block->m_row1);
            std::swap(block->m_row2, block->m_row3);
         }
      } else {
         for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC2Block*)((uint8_t*)sub.buff + y * sub.row_pitch_bytes);
            auto blocks1 = (BC2Block*)((uint8_t*)sub.buff + (numYBlocks - y - 1) * sub.row_pitch_bytes);
            for (uint32_t x = 0; x < numXBlocks; x++) {
               auto block0 = blocks0 + x;
               auto block1 = blocks1 + x;
               if (block0 != block1) {
                  std::swap(block0->m_alphaRow0, block1->m_alphaRow3);
                  std::swap(block0->m_alphaRow1, block1->m_alphaRow2);
                  std::swap(block0->m_alphaRow2, block1->m_alphaRow1);
                  std::swap(block0->m_alphaRow3, block1->m_alphaRow0);
                  std::swap(block0->m_color0, block1->m_color0);
                  std::swap(block0->m_color1, block1->m_color1);
                  std::swap(block0->m_row0, block1->m_row3);
                  std::swap(block0->m_row1, block1->m_row2);
                  std::swap(block0->m_row2, block1->m_row1);
                  std::swap(block0->m_row3, block1->m_row0);
               } else {
                  std::swap(block0->m_alphaRow0, block0->m_alphaRow3);
                  std::swap(block0->m_alphaRow1, block0->m_alphaRow2);
                  std::swap(block0->m_row0, block0->m_row3);
                  std::swap(block0->m_row1, block0->m_row2);
               }
            }
         }
      }
   }


   static void FlipBC3(const ddsktx_sub_data& sub) {
      struct BC3Block {
         uint8_t m_alpha0;
         uint8_t m_alpha1;
         uint8_t m_alphaR0;
         uint8_t m_alphaR1;
         uint8_t m_alphaR2;
         uint8_t m_alphaR3;
         uint8_t m_alphaR4;
         uint8_t m_alphaR5;
         uint16_t m_color0;
         uint16_t m_color1;
         uint8_t m_row0;
         uint8_t m_row1;
         uint8_t m_row2;
         uint8_t m_row3;
      };

      uint32_t numXBlocks = (sub.width + 3) / 4;
      uint32_t numYBlocks = (sub.height + 3) / 4;
      if (sub.height == 1) {
      } else if (sub.height == 2) {
         auto blocks = (BC3Block*)sub.buff;
         for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            uint8_t r0 = (block->m_alphaR1 >> 4) | (block->m_alphaR2 << 4);
            uint8_t r1 = (block->m_alphaR2 >> 4) | (block->m_alphaR0 << 4);
            uint8_t r2 = (block->m_alphaR0 >> 4) | (block->m_alphaR1 << 4);
            uint8_t r3 = (block->m_alphaR4 >> 4) | (block->m_alphaR5 << 4);
            uint8_t r4 = (block->m_alphaR5 >> 4) | (block->m_alphaR3 << 4);
            uint8_t r5 = (block->m_alphaR3 >> 4) | (block->m_alphaR4 << 4);

            block->m_alphaR0 = r0;
            block->m_alphaR1 = r1;
            block->m_alphaR2 = r2;
            block->m_alphaR3 = r3;
            block->m_alphaR4 = r4;
            block->m_alphaR5 = r5;
            std::swap(block->m_row0, block->m_row1);
            std::swap(block->m_row2, block->m_row3);
         }
      } else {
         for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC3Block*)((uint8_t*)sub.buff + y * sub.row_pitch_bytes);
            auto blocks1 = (BC3Block*)((uint8_t*)sub.buff + (numYBlocks - y - 1) * sub.row_pitch_bytes);
            for (uint32_t x = 0; x < numXBlocks; x++) {
               auto block0 = blocks0 + x;
               auto block1 = blocks1 + x;
               if (block0 != block1) {
                  std::swap(block0->m_alpha0, block1->m_alpha0);
                  std::swap(block0->m_alpha1, block1->m_alpha1);

                  uint8_t r0[6];
                  r0[0] = (block0->m_alphaR4 >> 4) | (block0->m_alphaR5 << 4);
                  r0[1] = (block0->m_alphaR5 >> 4) | (block0->m_alphaR3 << 4);
                  r0[2] = (block0->m_alphaR3 >> 4) | (block0->m_alphaR4 << 4);
                  r0[3] = (block0->m_alphaR1 >> 4) | (block0->m_alphaR2 << 4);
                  r0[4] = (block0->m_alphaR2 >> 4) | (block0->m_alphaR0 << 4);
                  r0[5] = (block0->m_alphaR0 >> 4) | (block0->m_alphaR1 << 4);
                  uint8_t r1[6];
                  r1[0] = (block1->m_alphaR4 >> 4) | (block1->m_alphaR5 << 4);
                  r1[1] = (block1->m_alphaR5 >> 4) | (block1->m_alphaR3 << 4);
                  r1[2] = (block1->m_alphaR3 >> 4) | (block1->m_alphaR4 << 4);
                  r1[3] = (block1->m_alphaR1 >> 4) | (block1->m_alphaR2 << 4);
                  r1[4] = (block1->m_alphaR2 >> 4) | (block1->m_alphaR0 << 4);
                  r1[5] = (block1->m_alphaR0 >> 4) | (block1->m_alphaR1 << 4);

                  block0->m_alphaR0 = r1[0];
                  block0->m_alphaR1 = r1[1];
                  block0->m_alphaR2 = r1[2];
                  block0->m_alphaR3 = r1[3];
                  block0->m_alphaR4 = r1[4];
                  block0->m_alphaR5 = r1[5];

                  block1->m_alphaR0 = r0[0];
                  block1->m_alphaR1 = r0[1];
                  block1->m_alphaR2 = r0[2];
                  block1->m_alphaR3 = r0[3];
                  block1->m_alphaR4 = r0[4];
                  block1->m_alphaR5 = r0[5];

                  std::swap(block0->m_color0, block1->m_color0);
                  std::swap(block0->m_color1, block1->m_color1);
                  std::swap(block0->m_row0, block1->m_row3);
                  std::swap(block0->m_row1, block1->m_row2);
                  std::swap(block0->m_row2, block1->m_row1);
                  std::swap(block0->m_row3, block1->m_row0);
               } else {
                  uint8_t r0[6];
                  r0[0] = (block0->m_alphaR4 >> 4) | (block0->m_alphaR5 << 4);
                  r0[1] = (block0->m_alphaR5 >> 4) | (block0->m_alphaR3 << 4);
                  r0[2] = (block0->m_alphaR3 >> 4) | (block0->m_alphaR4 << 4);
                  r0[3] = (block0->m_alphaR1 >> 4) | (block0->m_alphaR2 << 4);
                  r0[4] = (block0->m_alphaR2 >> 4) | (block0->m_alphaR0 << 4);
                  r0[5] = (block0->m_alphaR0 >> 4) | (block0->m_alphaR1 << 4);

                  block0->m_alphaR0 = r0[0];
                  block0->m_alphaR1 = r0[1];
                  block0->m_alphaR2 = r0[2];
                  block0->m_alphaR3 = r0[3];
                  block0->m_alphaR4 = r0[4];
                  block0->m_alphaR5 = r0[5];

                  std::swap(block0->m_row0, block0->m_row3);
                  std::swap(block0->m_row1, block0->m_row2);
               }
            }
         }
      }
   }


   static void FlipBC4(const ddsktx_sub_data& sub) {
      struct BC4Block {
         uint8_t m_red0;
         uint8_t m_red1;
         uint8_t m_redR0;
         uint8_t m_redR1;
         uint8_t m_redR2;
         uint8_t m_redR3;
         uint8_t m_redR4;
         uint8_t m_redR5;
      };

      uint32_t numXBlocks = (sub.width + 3) / 4;
      uint32_t numYBlocks = (sub.height + 3) / 4;
      if (sub.height == 1) {
      } else if (sub.height == 2) {
         auto blocks = (BC4Block*)sub.buff;
         for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            uint8_t r0 = (block->m_redR1 >> 4) | (block->m_redR2 << 4);
            uint8_t r1 = (block->m_redR2 >> 4) | (block->m_redR0 << 4);
            uint8_t r2 = (block->m_redR0 >> 4) | (block->m_redR1 << 4);
            uint8_t r3 = (block->m_redR4 >> 4) | (block->m_redR5 << 4);
            uint8_t r4 = (block->m_redR5 >> 4) | (block->m_redR3 << 4);
            uint8_t r5 = (block->m_redR3 >> 4) | (block->m_redR4 << 4);

            block->m_redR0 = r0;
            block->m_redR1 = r1;
            block->m_redR2 = r2;
            block->m_redR3 = r3;
            block->m_redR4 = r4;
            block->m_redR5 = r5;
         }
      } else {
         for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC4Block*)((uint8_t*)sub.buff + y * sub.row_pitch_bytes);
            auto blocks1 = (BC4Block*)((uint8_t*)sub.buff + (numYBlocks - y - 1) * sub.row_pitch_bytes);
            for (uint32_t x = 0; x < numXBlocks; x++) {
               auto block0 = blocks0 + x;
               auto block1 = blocks1 + x;
               if (block0 != block1) {
                  std::swap(block0->m_red0, block1->m_red0);
                  std::swap(block0->m_red1, block1->m_red1);

                  uint8_t r0[6];
                  r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                  r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                  r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                  r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                  r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                  r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);
                  uint8_t r1[6];
                  r1[0] = (block1->m_redR4 >> 4) | (block1->m_redR5 << 4);
                  r1[1] = (block1->m_redR5 >> 4) | (block1->m_redR3 << 4);
                  r1[2] = (block1->m_redR3 >> 4) | (block1->m_redR4 << 4);
                  r1[3] = (block1->m_redR1 >> 4) | (block1->m_redR2 << 4);
                  r1[4] = (block1->m_redR2 >> 4) | (block1->m_redR0 << 4);
                  r1[5] = (block1->m_redR0 >> 4) | (block1->m_redR1 << 4);

                  block0->m_redR0 = r1[0];
                  block0->m_redR1 = r1[1];
                  block0->m_redR2 = r1[2];
                  block0->m_redR3 = r1[3];
                  block0->m_redR4 = r1[4];
                  block0->m_redR5 = r1[5];

                  block1->m_redR0 = r0[0];
                  block1->m_redR1 = r0[1];
                  block1->m_redR2 = r0[2];
                  block1->m_redR3 = r0[3];
                  block1->m_redR4 = r0[4];
                  block1->m_redR5 = r0[5];

               } else {
                  uint8_t r0[6];
                  r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                  r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                  r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                  r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                  r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                  r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);

                  block0->m_redR0 = r0[0];
                  block0->m_redR1 = r0[1];
                  block0->m_redR2 = r0[2];
                  block0->m_redR3 = r0[3];
                  block0->m_redR4 = r0[4];
                  block0->m_redR5 = r0[5];
               }
            }
         }
      }
   }


   static void FlipBC5(const ddsktx_sub_data& sub) {
      struct BC5Block {
         uint8_t m_red0;
         uint8_t m_red1;
         uint8_t m_redR0;
         uint8_t m_redR1;
         uint8_t m_redR2;
         uint8_t m_redR3;
         uint8_t m_redR4;
         uint8_t m_redR5;
         uint8_t m_green0;
         uint8_t m_green1;
         uint8_t m_greenR0;
         uint8_t m_greenR1;
         uint8_t m_greenR2;
         uint8_t m_greenR3;
         uint8_t m_greenR4;
         uint8_t m_greenR5;
      };

      uint32_t numXBlocks = (sub.width + 3) / 4;
      uint32_t numYBlocks = (sub.height + 3) / 4;
      if (sub.height == 1) {
      } else if (sub.height == 2) {
         auto blocks = (BC5Block*)sub.buff;
         for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            uint8_t r0 = (block->m_redR1 >> 4) | (block->m_redR2 << 4);
            uint8_t r1 = (block->m_redR2 >> 4) | (block->m_redR0 << 4);
            uint8_t r2 = (block->m_redR0 >> 4) | (block->m_redR1 << 4);
            uint8_t r3 = (block->m_redR4 >> 4) | (block->m_redR5 << 4);
            uint8_t r4 = (block->m_redR5 >> 4) | (block->m_redR3 << 4);
            uint8_t r5 = (block->m_redR3 >> 4) | (block->m_redR4 << 4);

            block->m_redR0 = r0;
            block->m_redR1 = r1;
            block->m_redR2 = r2;
            block->m_redR3 = r3;
            block->m_redR4 = r4;
            block->m_redR5 = r5;

            uint8_t g0 = (block->m_greenR1 >> 4) | (block->m_greenR2 << 4);
            uint8_t g1 = (block->m_greenR2 >> 4) | (block->m_greenR0 << 4);
            uint8_t g2 = (block->m_greenR0 >> 4) | (block->m_greenR1 << 4);
            uint8_t g3 = (block->m_greenR4 >> 4) | (block->m_greenR5 << 4);
            uint8_t g4 = (block->m_greenR5 >> 4) | (block->m_greenR3 << 4);
            uint8_t g5 = (block->m_greenR3 >> 4) | (block->m_greenR4 << 4);

            block->m_greenR0 = g0;
            block->m_greenR1 = g1;
            block->m_greenR2 = g2;
            block->m_greenR3 = g3;
            block->m_greenR4 = g4;
            block->m_greenR5 = g5;
         }
      } else {
         for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC5Block*)((uint8_t*)sub.buff + y * sub.row_pitch_bytes);
            auto blocks1 = (BC5Block*)((uint8_t*)sub.buff + (numYBlocks - y - 1) * sub.row_pitch_bytes);
            for (uint32_t x = 0; x < numXBlocks; x++) {
               auto block0 = blocks0 + x;
               auto block1 = blocks1 + x;
               if (block0 != block1) {
                  std::swap(block0->m_red0, block1->m_red0);
                  std::swap(block0->m_red1, block1->m_red1);

                  uint8_t r0[6];
                  r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                  r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                  r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                  r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                  r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                  r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);
                  uint8_t r1[6];
                  r1[0] = (block1->m_redR4 >> 4) | (block1->m_redR5 << 4);
                  r1[1] = (block1->m_redR5 >> 4) | (block1->m_redR3 << 4);
                  r1[2] = (block1->m_redR3 >> 4) | (block1->m_redR4 << 4);
                  r1[3] = (block1->m_redR1 >> 4) | (block1->m_redR2 << 4);
                  r1[4] = (block1->m_redR2 >> 4) | (block1->m_redR0 << 4);
                  r1[5] = (block1->m_redR0 >> 4) | (block1->m_redR1 << 4);

                  block0->m_redR0 = r1[0];
                  block0->m_redR1 = r1[1];
                  block0->m_redR2 = r1[2];
                  block0->m_redR3 = r1[3];
                  block0->m_redR4 = r1[4];
                  block0->m_redR5 = r1[5];

                  block1->m_redR0 = r0[0];
                  block1->m_redR1 = r0[1];
                  block1->m_redR2 = r0[2];
                  block1->m_redR3 = r0[3];
                  block1->m_redR4 = r0[4];
                  block1->m_redR5 = r0[5];

                  std::swap(block0->m_green0, block1->m_green0);
                  std::swap(block0->m_green1, block1->m_green1);

                  uint8_t g0[6];
                  g0[0] = (block0->m_greenR4 >> 4) | (block0->m_greenR5 << 4);
                  g0[1] = (block0->m_greenR5 >> 4) | (block0->m_greenR3 << 4);
                  g0[2] = (block0->m_greenR3 >> 4) | (block0->m_greenR4 << 4);
                  g0[3] = (block0->m_greenR1 >> 4) | (block0->m_greenR2 << 4);
                  g0[4] = (block0->m_greenR2 >> 4) | (block0->m_greenR0 << 4);
                  g0[5] = (block0->m_greenR0 >> 4) | (block0->m_greenR1 << 4);
                  uint8_t g1[6];
                  g1[0] = (block1->m_greenR4 >> 4) | (block1->m_greenR5 << 4);
                  g1[1] = (block1->m_greenR5 >> 4) | (block1->m_greenR3 << 4);
                  g1[2] = (block1->m_greenR3 >> 4) | (block1->m_greenR4 << 4);
                  g1[3] = (block1->m_greenR1 >> 4) | (block1->m_greenR2 << 4);
                  g1[4] = (block1->m_greenR2 >> 4) | (block1->m_greenR0 << 4);
                  g1[5] = (block1->m_greenR0 >> 4) | (block1->m_greenR1 << 4);

                  block0->m_greenR0 = g1[0];
                  block0->m_greenR1 = g1[1];
                  block0->m_greenR2 = g1[2];
                  block0->m_greenR3 = g1[3];
                  block0->m_greenR4 = g1[4];
                  block0->m_greenR5 = g1[5];

                  block1->m_greenR0 = g0[0];
                  block1->m_greenR1 = g0[1];
                  block1->m_greenR2 = g0[2];
                  block1->m_greenR3 = g0[3];
                  block1->m_greenR4 = g0[4];
                  block1->m_greenR5 = g0[5];
               } else {
                  uint8_t r0[6];
                  r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                  r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                  r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                  r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                  r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                  r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);

                  block0->m_redR0 = r0[0];
                  block0->m_redR1 = r0[1];
                  block0->m_redR2 = r0[2];
                  block0->m_redR3 = r0[3];
                  block0->m_redR4 = r0[4];
                  block0->m_redR5 = r0[5];

                  uint8_t g0[6];
                  g0[0] = (block0->m_greenR4 >> 4) | (block0->m_greenR5 << 4);
                  g0[1] = (block0->m_greenR5 >> 4) | (block0->m_greenR3 << 4);
                  g0[2] = (block0->m_greenR3 >> 4) | (block0->m_greenR4 << 4);
                  g0[3] = (block0->m_greenR1 >> 4) | (block0->m_greenR2 << 4);
                  g0[4] = (block0->m_greenR2 >> 4) | (block0->m_greenR0 << 4);
                  g0[5] = (block0->m_greenR0 >> 4) | (block0->m_greenR1 << 4);

                  block0->m_greenR0 = g0[0];
                  block0->m_greenR1 = g0[1];
                  block0->m_greenR2 = g0[2];
                  block0->m_greenR3 = g0[3];
                  block0->m_greenR4 = g0[4];
                  block0->m_greenR5 = g0[5];
               }
            }
         }
      }
   }


   void TextureLoader::Flip(const uint32_t layer, const uint32_t slice, const uint32_t mipLevel) {
      auto info = static_cast<ddsktx_texture_info*>(m_Data);
      ddsktx_sub_data sub;
      ddsktx_get_sub(info, &sub, m_FileData.data(), m_FileData.size(), layer, slice, mipLevel);

      if (!IsCompressed()) {
         FlipNonCompressed(sub);
      } else if (info->format == DDSKTX_FORMAT_BC1) {
         FlipBC1(sub);
      } else if (info->format == DDSKTX_FORMAT_BC2) {
         FlipBC2(sub);
      } else if (info->format == DDSKTX_FORMAT_BC3) {
         FlipBC3(sub);
      } else if (info->format == DDSKTX_FORMAT_BC4) {
         FlipBC4(sub);
      } else if (info->format == DDSKTX_FORMAT_BC5) {
         FlipBC5(sub);
      }
   }

}