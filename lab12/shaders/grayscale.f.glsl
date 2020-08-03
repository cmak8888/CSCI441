#version 330 core

in vec2 texCoord;

uniform sampler2D fbo;

out vec4 fragColorOut;

void main() {
    vec4 texel = texture( fbo, texCoord );
    float avgRGB = (texel.x + texel.y + texel.z)/3.00;
    texel.r = avgRGB;
    texel.g = avgRGB;
    texel.b = avgRGB;
    vec4 finalColor = texel;
    
    // TODO #A

    fragColorOut = clamp(finalColor, 0.0, 1.0);
}
