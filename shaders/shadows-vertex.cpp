// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

#define SHADOWS_MOD

#include "shaders/vertexVersionSimple.h"

#include "shaders/uniformWorldConstants.h"

uniform highp float TIME;

attribute mediump vec4 POSITION;

void main()
{
	vec4 pos = POSITION;
	#ifdef SHADOWS_MOD
		float shadows = abs( sin( TIME / 250.0 - 10.0 ) + 0.76 );

		pos.y += shadows / 2.0;
		pos.y *= shadows * 2.0;
		pos.z += shadows / 2.0;
		pos.z *= shadows * 5.0;//10.0;
	#endif
    gl_Position = WORLDVIEWPROJ * pos;
}
