#version 450
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(set = 0,binding = 0, rgb10_a2) uniform image2D OutputImage;
layout(set = 1,binding = 0) uniform usampler2D ShadowImage;

//layout(push_constant) uniform Configuration{
//  vec4 ShadowColor;
//  vec2 ShadowOffset;
//} config;


void main() {
  ivec2 offset=ivec2(-40,-30);
  ivec2 coord=ivec2(gl_GlobalInvocationID.xy);
  ivec2 textureSize = textureSize(ShadowImage, 0);
  vec2 shadowUV=vec2((coord+offset) / vec2(textureSize));
  vec2 clipUV=vec2((coord) / vec2(textureSize));

  float shadow=texture(ShadowImage,shadowUV).r;
  float clip=texture(ShadowImage,clipUV).r;
  if(clip==0){
    if(shadow!=0)
      imageStore(OutputImage,coord,vec4(0.35,0.35,0.35,1.0));
  }
}