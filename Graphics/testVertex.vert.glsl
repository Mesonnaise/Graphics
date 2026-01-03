#version 450

// Input attributes from vertex buffer
layout(location = 0) in vec3 inPosition;
//layout(location = 2) in vec2 inTexCoord;


// Uniforms for transformation matrices
layout(set=0, binding=0) uniform SceneData {
  mat4 modelMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;
} camera;

// Output to fragment shader
//layout(location = 0) out vec3 fragPosition;
//layout(location = 1) out vec2 fragTexCoord;

void main() {
    // Transform vertex position from object space to world space
    vec4 worldPosition = camera.modelMatrix * vec4(inPosition, 1.0);
    
    // Transform from world space to view space (camera space)
    vec4 viewPosition = camera.viewMatrix * worldPosition;
    
    // Transform from view space to clip space (projection)
    gl_Position = camera.projectionMatrix * viewPosition;
    
    // Pass through world position for fragment shader calculations
    //fragPosition = worldPosition.xyz;
}