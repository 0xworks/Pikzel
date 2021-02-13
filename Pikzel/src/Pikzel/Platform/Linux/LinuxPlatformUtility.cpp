#include "Pikzel/Core/PlatformUtility.h"
#include "Pikzel/Core/Application.h"

//#include <commdlg.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>


namespace Pikzel {

   std::optional<std::filesystem::path> OpenFileDialog(const char* filter /*= nullptr*/) {
//   OPENFILENAMEA ofn;
//   CHAR filename[260] = {0};
//   ZeroMemory(&ofn, sizeof(OPENFILENAME));
//   ofn.lStructSize = sizeof(OPENFILENAME);
//   ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
//   ofn.lpstrFile = filename;
//   ofn.nMaxFile = sizeof(filename);
//   ofn.lpstrFilter = filter;
//   ofn.nFilterIndex = 1;
//   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
//   if (GetOpenFileNameA(&ofn) == TRUE) {
//      return ofn.lpstrFile;
//   }

      return std::nullopt;
   }


   std::optional<std::filesystem::path> SaveFileDialog(const char* filter /* = nullptr*/) {
//      OPENFILENAMEA ofn;
//      CHAR filename[260] = {0};
//      ZeroMemory(&ofn, sizeof(OPENFILENAME));
//      ofn.lStructSize = sizeof(OPENFILENAME);
//      ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
//      ofn.lpstrFile = filename;
//      ofn.nMaxFile = sizeof(filename);
//      ofn.lpstrFilter = filter;
//      ofn.nFilterIndex = 1;
//      ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
//
//      if (filter) {
//         ofn.lpstrDefExt = strchr(filter, '\0') + 1;
//      }
//
//      if (GetSaveFileNameA(&ofn) == TRUE) {
//         return ofn.lpstrFile;
//      }

      return std::nullopt;
   }

}
