#version 450

struct BlobLayerConfig{
 vec4 Color;
 uint VertexOffset;
 uint VertexCount;
 vec2 Padding;
};

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(set = 0, binding = 0, rgba32f) uniform image2D OutputImage;
layout(set = 1, binding = 0, std430) buffer VertexData{
  vec4 vertices[128];
} vertexData;



layout(push_constant) uniform Configuration{
  vec4 Color[4];
  uint Count[4];
} config;

float BlobLayer(uint offset,uint count){
  ivec2 coord=ivec2(gl_GlobalInvocationID.xy);
  float field=0.0;

  for(uint i=0;i<count;i++){
    vec4 vertex=vertexData.vertices[i+offset];
    float d=distance(coord,vertex.xy);
    float r=vertex.z;
    field+=(r*r)/(d*d);
  }

  field=smoothstep(1.0 - 0.005,1.0 + 0.005, field);
  field=clamp(field,0.0,1.0);

  return field;
}

void main(){
  ivec2 coord=ivec2(gl_GlobalInvocationID.xy);
  vec4 outColor=config.Color[0];

  uint vertexOffset=0;
  for(uint i=0;i<3;i++){
    float field=BlobLayer(vertexOffset,config.Count[i]);
    vertexOffset+=config.Count[i];

    outColor=mix(outColor,config.Color[i+1],field);
  }
  imageStore(OutputImage,coord,outColor);
}