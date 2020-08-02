#version 330 core

in vec2 texCoord;

out vec4 fragColorOut;

uniform sampler2D tex;
uniform vec4 color;

void main() {
  fragColorOut = texture( tex, texCoord ) * color;
}
