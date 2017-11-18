// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

#define LIGHTNING_CORRECTION

#include "shaders/fragmentVersionSimple.h"
uniform highp float TIME;

varying vec4 color;

void main()
{
	vec4 c = color;
	#ifdef LIGHTNING_CORRECTION
		c.rg *= 0.5; //Lightning color correction
		c.a *= (sin(TIME * 20.0) + 1.75) * 0.9; //Lightning smooth gradient
	#endif
	gl_FragColor = c;
}
