// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

#define LIGHTNING_WAVES

#include "shaders/vertexVersionSimple.h"

#include "shaders/uniformWorldConstants.h"

uniform highp float TIME;

attribute mediump vec4 POSITION;
attribute vec4 COLOR;

varying vec4 color;

void main()
{
	vec4 pos = POSITION;
	#ifdef LIGHTNING_WAVES
		pos.x += sin(TIME * 20.0 + pos.y / 2.0) * 0.9; //X waves
		pos.z += sin(TIME * 22.0 + pos.y / 3.0) * 0.9; //Z waves
	#endif
    gl_Position = WORLDVIEWPROJ * pos;

    color = COLOR;
}
