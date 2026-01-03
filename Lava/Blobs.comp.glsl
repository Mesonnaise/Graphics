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
  BlobLayerConfig Layers[4];
  vec4 vertices[128];
} vertexData;



layout(push_constant) uniform Configuration{
  vec4 ClearColor;
  vec2 Resolution;
  float Threshold;
  uint LayerCount;
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

  field=smoothstep(config.Threshold - 0.005, config.Threshold + 0.005, field);
  field=clamp(field,0.0,1.0);

  return field;
}

void main(){
  ivec2 coord=ivec2(gl_GlobalInvocationID.xy);
  vec4 outColor=config.ClearColor;

  for(uint i=0;i<config.LayerCount;i++){
    BlobLayerConfig blobConfig=vertexData.Layers[i];

    float field=BlobLayer(blobConfig.VertexOffset,blobConfig.VertexCount);
    outColor=mix(outColor,blobConfig.Color,field);
  }
  imageStore(OutputImage,coord,outColor);
}