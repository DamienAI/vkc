#version 440

layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;

layout(push_constant) uniform Input {
  uint elementsCount;
} inParams;

layout(std430, binding = 0) buffer lay1 { uint y[]; };

uint computeGlobalInvocationIndex()
{
  const uvec3 sizes = gl_NumWorkGroups * gl_WorkGroupSize;

  uint zOffset = gl_GlobalInvocationID.z * sizes.x * sizes.y;
  uint yOffset = gl_GlobalInvocationID.y * sizes.x;
  return zOffset + yOffset + gl_GlobalInvocationID.x;
}

void main()
{
  const uint globalInvocationIndex = computeGlobalInvocationIndex();
  if(globalInvocationIndex >= inParams.elementsCount){
    return;
  }
  atomicAdd(y[0], 1U);
}