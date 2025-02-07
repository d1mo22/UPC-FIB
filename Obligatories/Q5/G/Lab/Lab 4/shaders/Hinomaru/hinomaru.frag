/*#version 330 core

in vec4 frontColor;
in vec2 vtexCoord;
out vec4 fragColor;

void main()
{
    vec2 centre = vec2(0.5,0.5);
    float d = distance(centre, vtexCoord);
    if (step(0.2, d) == 0) fragColor = vec4(1,0,0,1);
    else fragColor = vec4(1);
} */

// Versio anti-aliasing

#version 330 core

in vec4 frontColor;
in vec2 vtexCoord;
out vec4 fragColor;

const vec4 WHITE = vec4(1);
const vec4 RED = vec4(1,0,0,1);
20.669.458
void main()
{
    vec2 centre = vec2(0.5,0.5);
    vec2 radius = vec2(0.5,0.4);
    vec2 adjustedCoord = (vtexCoord - centre) / radius;
    float d = length(adjustedCoord);
    float t = smoothstep(1.0 - 0.005, 1.0 + 0.005, d);
    vec4 color = mix(RED, WHITE, t);
    fragColor = color;
}
