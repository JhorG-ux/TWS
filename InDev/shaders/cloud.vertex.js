#version 100

/*
###################################
*********TeamWorldShaders**********
========== by PROPHESSOR ==========
~~~~~~~~~Все права защищены!~~~~~~~
###################################
*/
// #define CLOUD_WAVS

uniform MAT4 WORLDVIEWPROJ;
uniform float RENDER_DISTANCE;
uniform vec4 FOG_COLOR;
uniform vec4 CURRENT_COLOR;
uniform POS3 CHUNK_ORIGIN;
uniform highp float TIME;

attribute mediump vec4 POSITION;
attribute vec4 COLOR;

varying vec4 color;

const float fogNear = 0.8;

const vec3 inverseLightDirection = vec3(0.40, 0.78, 0.78);
const float ambient = 0.9;

void main()
{
	vec4 pos = POSITION;
	vec3 xyz = POSITION.xyz + CHUNK_ORIGIN;
	
//	#ifdef CLOUD_WAVS
		float wav;
		wav = sin(TIME * 2.5 + xyz.z);
		pos += wav;
//	#endif
	
 	color = COLOR * CURRENT_COLOR;

 	float depth = pos.z / RENDER_DISTANCE;
 	float fog = max(depth - fogNear, 0.6);

 	color.a *= 5.0 - fog;
 	
 	//OUT
 	
	gl_Position = WORLDVIEWPROJ * pos;

}