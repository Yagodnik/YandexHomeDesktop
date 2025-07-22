#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float t;
};

layout(binding = 1) uniform sampler2D source;

void main() {
    vec4 p = texture(source, qt_TexCoord0);
    if (p.a < 0.01) {
        discard;
    }

    vec4 baseColor = vec4(0.85, 0.85, 0.85, 0.9);

    float band = smoothstep(0.0, 1.0, qt_TexCoord0.x);
    float shimmer = exp(-pow((qt_TexCoord0.x - t) * 5.0, 2.0)) * 0.03;

    vec4 finalColor = baseColor + vec4(shimmer, shimmer, shimmer, 0.0);
    fragColor = finalColor * qt_Opacity;
}
