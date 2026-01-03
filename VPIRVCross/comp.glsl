#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// Input buffer - read-only data
layout(std430, binding = 0) readonly buffer InputBuffer {
    uint input_data[];
};

// Output buffer - write-only data
layout(std430, binding = 1) writeonly buffer OutputBuffer {
    uint output_data[];
};

// Uniform buffer for parameters (optional)
layout(binding = 2) uniform UniformBuffer {
    uint data_size;
    uint some_constant;
};

void main(){
    // Calculate global work item index
    uint global_id = gl_GlobalInvocationID.x + 
                     gl_GlobalInvocationID.y * gl_WorkGroupSize.x;
    
    // Check bounds
    if (global_id >= data_size) {
        return;
    }
    
    // Perform some computation
    uint value = input_data[global_id];
    uint result = value * some_constant + global_id;
    
    // Write to output buffer
    output_data[global_id] = result;
}