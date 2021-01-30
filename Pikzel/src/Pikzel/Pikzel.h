#pragma once

// Master include file to bring in all of Pikzel
// If you prefer, you can bring in just the bits that you know you will use.
// (for possibly faster compile times - your mileage may vary)

#include "Pikzel/Core/Application.h"
#include "Pikzel/Core/PlatformUtility.h"
#include "Pikzel/Core/Utility.h"

#include "Pikzel/Events/ApplicationEvents.h"
#include "Pikzel/Events/EventDispatcher.h"
#include "Pikzel/Events/KeyEvents.h"
#include "Pikzel/Events/WindowEvents.h"

#include "Pikzel/ImGui/ImGuiEx.h"

#include "Pikzel/Input/Input.h"
#include "Pikzel/Input/KeyCodes.h"
#include "Pikzel/Input/MouseButtons.h"

#include "Pikzel/Renderer/Buffer.h"
#include "Pikzel/Renderer/ComputeContext.h"
#include "Pikzel/Renderer/Framebuffer.h"
#include "Pikzel/Renderer/GraphicsContext.h"
#include "Pikzel/Renderer/MeshRenderer.h"
#include "Pikzel/Renderer/ModelRenderer.h"
#include "Pikzel/Renderer/Pipeline.h"
#include "Pikzel/Renderer/RenderCore.h"
#include "Pikzel/Renderer/sRGB.h"
#include "Pikzel/Renderer/Texture.h"

#include "Pikzel/Scene/Camera.h"
#include "Pikzel/Scene/Light.h"
#include "Pikzel/Scene/Mesh.h"
#include "Pikzel/Scene/Model.h"
#include "Pikzel/Scene/ModelSerializer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
