#version 400
uniform sampler2DRect input;
uniform sampler2DRect effect;

uniform vec2 p0;
uniform vec2 p1;
uniform vec2 resolution;
uniform float alpha;

in vec2 vTexCoord;
out vec4 outputColor;

void main() {
    vec3 inputColor = texture(input, vTexCoord).rgb;

    vec2 size = textureSize(effect);
    vec3 effectColor = texture(effect, vTexCoord / resolution * size).rgb;

    vec3 color0 = inputColor;
    vec3 color1 = effectColor;

    float a = step(p0.x, vTexCoord.x) - step(p1.x, vTexCoord.x);
    float b = step(p0.y, vTexCoord.y) - step(p1.y, vTexCoord.y);

    vec3 target = mix(color0, color1, alpha);

    outputColor = vec4(mix(color0, target, a * b), 1.0);
}