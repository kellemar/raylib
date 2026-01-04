#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;

void main()
{
    vec2 uv = fragTexCoord;

    // Barrel distortion
    vec2 center = uv - 0.5;
    float dist = dot(center, center);
    uv = uv + center * dist * 0.1;

    // Check if we're outside the texture after distortion
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Scanlines
    float scanline = sin(uv.y * 720.0 * 3.14159) * 0.03;

    // RGB offset (chromatic aberration)
    float offset = 0.002;
    float r = texture(texture0, uv + vec2(offset, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(offset, 0.0)).b;

    // Vignette
    float vignette = 1.0 - dist * 1.2;
    vignette = clamp(vignette, 0.0, 1.0);

    // Flicker (subtle)
    float flicker = 1.0 - sin(time * 10.0) * 0.01;

    vec3 color = vec3(r, g, b) * vignette * flicker;
    color -= scanline;

    finalColor = vec4(color, 1.0);
}
