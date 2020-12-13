#define MAX_POINT_LIGHTS 32

struct PointLight {
   vec3 position;
   vec3 color;
   float power;
};
