#version 450
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 0,binding = 0, rgba32f) uniform image2D OutputImage;
//layout(set = 1,binding = 0) uniform sampler2D ShadowImage;
layout(set = 1,binding = 0, rg32f) uniform readonly image2D ShadowImage;

layout(push_constant) uniform Configuration{
  vec4 ShadowColor;
  vec2 ShadowOffset;
} config;

//void main() {
//  ivec2 coord=ivec2(gl_GlobalInvocationID.xy);
//  ivec2 textureSize = textureSize(ShadowImage, 0);
//  vec2 uv=vec2(coord) / vec2(textureSize);
//
//  float value=texture(ShadowImage,uv).r;
//  if(value==0) 
//    imageStore(OutputImage,coord,vec4(1.0,0.0,1.0,1.0));
//  else
//    imageStore(OutputImage,coord,vec4(1.0,1.0,1.0,1.0));
//}

void main() {
  ivec2 coord=ivec2(gl_GlobalInvocationID.xy);

  float value=imageLoad(ShadowImage,coord).g;
  imageStore(OutputImage,coord,vec4(value,value,value,1.0));
}