#version 410
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;
// layout(location = 3) in vec3 inTangent;
// layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec2 vsTexcoord;

uniform sampler2D texHeight;

float getHeight(vec2 uv) { return texture(texHeight, uv, 0).r * 1000; }

void main() {
    vec3 vsPos = inPos;
    // vsPos.y = getHeight(inTexcoord);
    vsTexcoord = inTexcoord;
    gl_Position = vec4(vsPos, 1);
}