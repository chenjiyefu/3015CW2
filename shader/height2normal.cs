#version 450
layout(local_size_x = 1024) in;

// height tex
layout(binding = 0, rgba32f) uniform readonly image2D inTex;
layout(binding = 1, rgba8) uniform writeonly image2D outTex;

float getHeight(ivec2 xy) { return imageLoad(inTex, xy).r * 100000; }

vec3 calcNormal(ivec2 xy) {
    int s = 1;
    float h1 = getHeight(xy + ivec2(-s));
    float h2 = getHeight(xy + ivec2(0, -s));
    float h3 = getHeight(xy + ivec2(s, -s));
    float h4 = getHeight(xy + ivec2(-s, 0));
    float h5 = getHeight(xy + ivec2(0, 0));
    float h6 = getHeight(xy + ivec2(s, 0));
    float h7 = getHeight(xy + ivec2(-s, s));
    float h8 = getHeight(xy + ivec2(0, s));
    float h9 = getHeight(xy + ivec2(s, s));

    float dh0 = h3 + 2 * h6 + h9 - (h1 + 2 * h4 + h7);
    float dh1 = h7 + 2 * h8 + h9 - (h1 + 2 * h2 + h3);

    //1 texel 1 meter
    // vec3 scaleFactor = vec3(1.);
    vec3 v0 = vec3(8 * s, 0, dh0);
    vec3 v1 = vec3(0, 8 * s, dh1);
    return normalize(cross(v0, v1));
    // return v1;
}

void main() {
    ivec2 imgSize = imageSize(inTex);
    const uint w = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    const uint h = gl_NumWorkGroups.y * gl_WorkGroupSize.y;
    const uvec3 c = uvec3(1, w, w * h);
    uint globalInvocationIndex = uint(dot(c, gl_GlobalInvocationID));

    ivec2 st = ivec2(globalInvocationIndex % imgSize.x, globalInvocationIndex / imgSize.x);
    if (st.y >= imgSize.y) return;
    vec3 n = calcNormal(st);
    imageStore(outTex, st, vec4(n * 0.5 + 0.5, 1));
}