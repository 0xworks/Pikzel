#define MAX_POINT_LIGHTS 32

struct DirectionalLight {
   vec3 direction;
   vec3 color;
   vec3 ambient;
};

struct PointLight {
   vec3 position;
   vec3 color;
   float power;
};
