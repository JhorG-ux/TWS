// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.


//Конфигурация
//#define NIGHT_VISION //Ночное зрение
//#define COLOR_FILTER //Цветовой фильтр
//#define TORCH_BLINK //Дрожание света от факела

//Тестирование
#define TEST_UV

#include "shaders/fragmentVersionCentroid.h"

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

vec4 TorchProcessor(vec4 color){
	return color;
}

void main(){
	//Заглушка
	#ifdef BYPASS_PIXEL_SHADER
		gl_FragColor = vec4(0, 0, 0, 0);
		return;
	#else 

	vec4 diffuse, lights;

	#if !defined(TEXEL_AA) || !defined(TEXEL_AA_FEATURE)
		diffuse = texture2D( TEXTURE_0, uv0 );
		lights  = texture2D( TEXTURE_1, uv1 );
	#else
		diffuse = texture2D_AA(TEXTURE_0, uv0 );
		lights  = texture2D( TEXTURE_1, uv1 );
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
		#ifndef NIGHT_VISION
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