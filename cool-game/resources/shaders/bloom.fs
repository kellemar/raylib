#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float intensity;

void main()
{
    vec4 color = texture(texture0, fragTexCoord);

    // Extract bright parts
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec4 bright = (brightness > 0.6) ? color : vec4(0.0);

    // Blur for bloom effect
    vec4 blur = vec4(0.0);
    float blurSize = 0.004;

    for (int x = -3; x <= 3; x++)
    {
        for (int y = -3; y <= 3; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * blurSize;
            vec4 texSample = texture(texture0, fragTexCoord + offset);
            float sampleBrightness = dot(texSample.rgb, vec3(0.2126, 0.7152, 0.0722));
            if (sampleBrightness > 0.5)
            {
                blur += texSample;
            }
        }
    }
    blur /= 49.0;

    // Combine original with bloom
    finalColor = color + blur * intensity;
    finalColor.a = 1.0;
}
