#version 440

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (constant_id = 0) const float floatValue = 0.0f;
layout (constant_id = 1) const uint uintValue = 0;
layout (constant_id = 2) const bool boolValue = false;

layout(std430, binding = 0) buffer lay0 { float y[]; };

void main(){
  y[0] = floatValue;
  y[1] = float(uintValue);
  y[2] = boolValue ? 42.0f : -42.0f;
}