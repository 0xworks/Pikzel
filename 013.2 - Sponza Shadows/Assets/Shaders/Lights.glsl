#define MAX_POINT_LIGHTS 4

struct DirectionalLight {
   vec3 direction;
   vec3 color;
   vec3 ambient;
   float size;
};

struct PointLight {
   vec3 position;
   vec3 color;
   float size;
   float power;
};
