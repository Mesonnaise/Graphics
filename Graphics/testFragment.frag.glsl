#version 450


// Uniforms for transformation matrices
layout(set=0, binding=0) uniform SceneData {
  mat4 modelMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;
} camera;

// Output color
layout(location = 0) out vec4 fragColor;

void main() {
    // Apply the solid color from uniform buffer
    fragColor = vec4(0.0,1.0,1.0,1.0);
    
}