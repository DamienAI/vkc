#version 440

layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;

layout(std430, binding = 0) buffer lay0 { uint y[]; };

void main(){
  atomicAdd(y[0], 1U);
}