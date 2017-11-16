// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

#include "shaders/fragmentVersionCentroid.h"
// ????????
#if __VERSION__ >= 300
	#ifndef BYPASS_PIXEL_SHADER
		#if defined(TEXEL_AA) && defined(TEXEL_AA_FEATURE)
			_centroid in highp vec2 uv0;
			_centroid in highp vec2 uv1;
		#else
			_centroid in vec2 uv0;
			_centroid in vec2 uv1;
		#endif
	#endif
#else
	#ifndef BYPASS_PIXEL_SHADER
		varying vec2 uv0;
		varying vec2 uv1;
	#endif
#endif

//Конфигурация
#define NIGHT_VISION //Отключить тени


varying vec4 color;

#ifdef FOG
varying vec4 fogColor;
#endif

#ifdef NEAR_WATER
varying float cameraDist;
#endif

#include "shaders/uniformShaderConstants.h"
#include "shaders/util.h"

uniform sampler2D TEXTURE_0;
uniform sampler2D TEXTURE_1;
uniform sampler2D TEXTURE_2;

#define saturation 1.510
#define exposure 1.1
#define brightness 0.600
#define gamma 1.100
#define contrast 1.997

float filmic_curve(float x) {

	// Shoulder strength
	float A = 0.22;
	// Linear strength
	float B = 0.5;
	// Linear angle
	float C = 0.15 * brightness;
	// Toe strength
	float D = 0.4 * gamma;
	// Toe numerator
	float E = 0.01 * contrast;
	// Toe denominator
	float F = 0.2;

	return ((x * (A * x + C * B) + D * E) / 
			(x * (A * x + B) + D * F)) - E / F;

}


void main(){
	//Заглушка
#ifdef BYPASS_PIXEL_SHADER
	gl_FragColor = vec4(0, 0, 0, 0);
	return;
#else 

	#if !defined(TEXEL_AA) || !defined(TEXEL_AA_FEATURE)
		vec4 diffuse = texture2D( TEXTURE_0, uv0 );
		vec4 lights  = texture2D( TEXTURE_1, uv1 );
	#else
		vec4 diffuse = texture2D_AA(TEXTURE_0, uv0 );
		vec4 lights  = texture2D( TEXTURE_1, uv1 );
	#endif

	vec4 inColor = color;
	
	#ifdef SEASONS_FAR
		diffuse.a = 1.0;
		inColor.b = 1.0;
	#endif

	#ifdef ALPHA_TEST
		#ifdef ALPHA_TO_COVERAGE
			float alphaThreshold = .05;
		#else
			float alphaThreshold = .5;
		#endif
		if(diffuse.a < alphaThreshold)
			discard;
	#endif
	
	#if !defined(ALWAYS_LIT)
		#ifdef NIGHT_VISION
		#else
			diffuse = diffuse * lights; //Применяется освещение
		#endif
	#endif

	#ifndef SEASONS

		#if !defined(ALPHA_TEST) && !defined(BLEND)
			diffuse.a = inColor.a;
		#elif defined(BLEND)
			#ifdef NEAR_WATER
				//vec3 waterColor = vec3(0.7,0.2,0.5); //Цвет воды
				//diffuse.rgb = waterColor;
				diffuse.a *= inColor.a * 0.5; //Соответствие воды окружению
				//float alphaFadeOut = clamp(cameraDist, 0.0, 1.0);
				//diffuse.a = mix(diffuse.a, 1.0, alphaFadeOut);
			#endif
	    #endif	
	
		diffuse.rgb *= inColor.rgb;
	#else
		vec2 uv = inColor.xy;
		//Чё
		diffuse.rgb *= mix(vec3(1.0,1.0,1.0), texture2D( TEXTURE_2, uv).rgb*2.0, inColor.b);
		diffuse.rgb *= inColor.aaa;
		diffuse.a = 1.0;
	#endif

	#ifdef FOG
		diffuse.rgb = mix( diffuse.rgb, fogColor.rgb, fogColor.a );
	#endif
	
	
	gl_FragColor = diffuse;

#endif // BYPASS_PIXEL_SHADER
}


/*
// Отсеивание
// Реагирует на края лавы и воды
if(uv1.y < 0.875){s_amount = 0.05;}
if(uv1.y < 0.874){s_amount = 0.10;}
if(uv1.y < 0.873){s_amount = 0.15;}
if(uv1.y < 0.872){s_amount = 0.20;}
if(uv1.y < 0.871){s_amount = 0.25;}
if(uv1.y < 0.870){s_amount = 0.30;}
if(uv1.y < 0.869){s_amount = 0.35;}
if(uv1.y < 0.868){s_amount = 0.40;}
if(uv1.y < 0.867){s_amount = 0.45;}

*/
