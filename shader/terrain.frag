#version 410
out vec4 fragColor;

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec4 inPositionShadow;

uniform sampler2D baseColorTex;
uniform sampler2D texNormal;
uniform sampler2DShadow texShadow;

uniform vec3 lightDirection;

void main() {
    vec3 p = inPositionShadow.xyz / inPositionShadow.w * 0.5 + 0.5;
    float shadow = texture(texShadow, p);
    // fragColor = vec4(p.xy, 0, 1);
    // fragColor = vec4(vec3(shadow), 1);
    // return;

    vec2 uv = inTexcoord;
    vec4 albedo = texture(baseColorTex, inTexcoord);

    mat3 m = mat3(1, 0, 0, 0, 0, -1, 0, 1, 0);

    vec3 N = m * (texture(texNormal, inTexcoord, 0).xyz * 2. - 1.);

    float NdotL = dot(N, lightDirection);

    float ambient = 0.1;
    albedo.rgb = pow(albedo.rgb, vec3(2.2));
    vec3 col = albedo.rgb * (shadow * max(NdotL, 0.) + ambient);
    fragColor = vec4(col, 1);
}