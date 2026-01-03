#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform COLOR { vec4 value; } in1;
layout(set = 1, binding = 0, rgba32f) uniform image2D OutputImage;

void main() {
  uvec3 gid = gl_GlobalInvocationID;            // global invocation ID
  ivec2 coord = ivec2(gid.xy);                  // 2D image coords
  imageStore(OutputImage, coord, in1.value);
}