// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

#include "shaders/uniformShaderConstants.h"
#include "shaders/util.h"

uniform sampler2D TEXTURE_0;

varying vec4 color;
varying vec4 fragCoord;

uniform float TIME;

void main()
{
    vec4 diffuse = texture2D(TEXTURE_0, vec2(fragCoord.x * 10000.0 + TIME, fragCoord.z * 10000.0));

    gl_FragColor = diffuse;
}
