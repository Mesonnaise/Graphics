#version 450


layout(location = 0) in vec3 inPosition;
layout(set = 0, binding = 0) uniform Matrices {
	mat4 model;
	mat4 view;
	mat4 proj;
} Mat;

void main(){
	// Transform vertex position from object space to world space
	vec4 worldPosition = Mat.model * vec4(inPosition, 1.0);
    
	// Transform from world space to view space (camera space)
	vec4 viewPosition = Mat.view * worldPosition;
    
	// Transform from view space to clip space (projection)
	gl_Position = Mat.proj * viewPosition;
}