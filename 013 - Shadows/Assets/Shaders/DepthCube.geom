#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

layout(set = 0, binding = 0) uniform UBOLightViews {
   mat4 views[6];
} light;

layout (location = 0) out vec4 outFragPos;

void main() {
   for(int face = 0; face < 6; ++face) {
      gl_Layer = face;
      for(int i = 0; i < 3; ++i) {
         outFragPos = gl_in[i].gl_Position;
         gl_Position = light.views[face] * outFragPos;
         EmitVertex();
      }
      EndPrimitive();
   }
}
