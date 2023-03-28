#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    for (float x = 0.01; x < 1; x--) {
         outColor = vec4(1.0f, x*0.5f, 0.2f, 1.0f);
    }
}
