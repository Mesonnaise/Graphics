#version 450

// Input from vertex shader
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out vec3 fragNormal;
layout(location = 5) out vec3 fragWorldPos;

// Uniforms
layout(set=0, binding=0) uniform ColorData{
  vec3 lightPosition;
  vec3 cameraPosition;
  vec3 lightColor;
} colorUBO;

layout(set=1,binding=0) uniform Duplicate{
  vec3 simpleVex;
} dupUBO;

layout(set=0, binding=1) uniform sampler2D diffuseTexture;


// Output color
layout(location = 6) out vec4 fragColor;

void main() {
    // Sample texture
    vec4 texColor = texture(diffuseTexture, fragTexCoord);
    
    // Skip fully transparent pixels (optional)
    if (texColor.a < 0.1) {
        discard;
    }
    
    // Normalize normals
    vec3 normal = normalize(fragNormal);
    
    // Calculate lighting
    vec3 lightDir = normalize(colorUBO.lightPosition - fragWorldPos);
    vec3 viewDir = normalize(colorUBO.cameraPosition - fragWorldPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    
    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * colorUBO.lightColor * dupUBO.simpleVex;
    
    // Specular lighting (optional)
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * colorUBO.lightColor;
    
    // Combine lighting with texture
    vec3 result = (diffuse + specular) * texColor.rgb;
    
    // Output final color
    fragColor = vec4(result, texColor.a);
}