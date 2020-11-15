#version 440

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

layout(std430, binding = 0) buffer lay0 { uint y[]; };

void main(){
  atomicAdd(y[0], 1U);

  const uint globalIndex = gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_GlobalInvocationID.x;
  
  uint offset = 4 * globalIndex + 1;
  y[offset++] = globalIndex;
  switch (globalIndex % 4) {
    case 0:
      y[offset++] = gl_LocalInvocationID.x;
      y[offset++] = gl_LocalInvocationID.y;
      y[offset++] = gl_LocalInvocationID.z;
    break;
    case 1:
      y[offset++] = gl_GlobalInvocationID.x;
      y[offset++] = gl_GlobalInvocationID.y;
      y[offset++] = gl_GlobalInvocationID.z;
    break;
    case 2:
      y[offset++] = gl_NumWorkGroups.x;
      y[offset++] = gl_NumWorkGroups.y;
      y[offset++] = gl_NumWorkGroups.z;
    break;
    case 3:
      y[offset++] = gl_WorkGroupID.x;
      y[offset++] = gl_WorkGroupID.y;
      y[offset++] = gl_WorkGroupID.z;
    break;
    default:
      y[offset++] = 42;
      y[offset++] = 42;
      y[offset++] = 42;
      break;
  }
}