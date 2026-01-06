#version 450


layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform Matrices {
	mat4 model;
	mat4 view;
	mat4 proj;
} Mat;


void main(){
  if(gl_PrimitiveID % 4 == 0||gl_PrimitiveID % 4 == 1 )
	fragColor = vec4(1.0,0.0,0.0,1.0);
  else
    fragColor = vec4(1.0,1.0,1.0,1.0);
}