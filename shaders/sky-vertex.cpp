// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

//#define INVERT_RB
#define GRAY_FILTER
//#define TEST

#include "shaders/vertexVersionSimple.h"

#include "shaders/uniformWorldConstants.h"
#include "shaders/uniformPerFrameConstants.h"
#include "shaders/uniformShaderConstants.h"

attribute mediump vec4 POSITION;
attribute vec4 COLOR;

varying vec4 color;

const float fogNear = 0.3;

float calc_weather_value(){
	return 0.0;
}

void main()
{
    gl_Position = WORLDVIEWPROJ * POSITION;
	vec4 mixval = CURRENT_COLOR;
	
	#ifdef INVERT_RB
		mixval.r = CURRENT_COLOR.b;
		mixval.b = CURRENT_COLOR.r;
	#endif
	
	#ifdef CUSTOM_NIGHT
		mixval.r = max(mixval.r, NIGHT_R);
		mixval.g = max(mixval.g, NIGHT_G);
		mixval.b = max(mixval.b, NIGHT_B);
	#endif
	
	#ifdef GRAY_FILTER
		vec4 gray = vec4(mixval.r, mixval.r, mixval.r, mixval.a);
		mixval = mix(mixval, gray, 0.0);
	#endif
	
    color = mix( mixval, FOG_COLOR, COLOR.r );
	
	#ifdef TEST
		color = vec4(0.0,1.0,0.0,1.0);
	#endif
}
