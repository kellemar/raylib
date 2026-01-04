#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float intensity;  // 0.0 = no effect, 1.0 = max effect
uniform float time;

void main()
{
    vec2 uv = fragTexCoord;

    // Pulsing effect - oscillates between 0.5x and 1.5x intensity
    float pulse = 1.0 + 0.5 * sin(time * 8.0);

    // Base offset scales with intensity and pulse
    // Max offset of 0.015 at full intensity for dramatic effect
    float offset = intensity * pulse * 0.015;

    // Direction from center for radial aberration
    vec2 dir = uv - 0.5;
    float dist = length(dir);

    // Aberration stronger toward edges
    float edgeFactor = 1.0 + dist * 2.0;
    offset *= edgeFactor;

    // Sample RGB channels with offset
    float r = texture(texture0, uv + dir * offset).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - dir * offset).b;

    finalColor = vec4(r, g, b, 1.0);
}
