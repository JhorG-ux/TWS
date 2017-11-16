// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.


//Конфигурация
//#define NIGHT_VISION //Ночное зрение
//#define COLOR_FILTER //Цветовой фильтр
//#define TORCH_BLINK //Дрожание света от факела

//Тестирование
// #define TEST_UV
#define UsernameAKs_Lights
//#define SHADOWS
//#define DYNAMIC_SHADOWS
//#define TORCH_PROCESSOR

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

vec4 TorchProcessor(vec4 _color, vec4 _light){
	vec4 color = _color;
	vec4 light = _light;
	light.rgb *= color.rgb*0.75;
	light.r *= 1.0;
	light.g *= 0.65;
	light.b *= 0.101;
	light.rgb *= 3.0;
	color *= color*light;
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
	
	#ifdef TORCH_PROCESS
		diffuse = TorchProcessor(diffuse, lights);
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

	#ifdef UsernameAKs_Lights	
		lights.rgb *= color.rgb*0.75;
		lights.r *= 0.5;
		lights.g *= 0.3;
		lights.b *= 0.0;
//		lights.rgb *= 1.5;
		diffuse *= /*color**/lights;
	#endif

	#ifdef SHADOWS
		if(diffuse.r > 2.1 && diffuse.g > 2.1 && diffuse.b > 2.1) {
			//diffuse.r *= 1.05;
			diffuse.b = 1.0;
		}
		if(diffuse.b > 0.2) {
			diffuse.a = 0.008;
		}
		
		if(uv1.y < 0.9068) {
		    diffuse.r = 1.0;// *= 0.7*1.0+ uv1.x *0.7857; //0.66
		    //diffuse.g *= 0.7*1.0+ uv1.x *0.6428; //0.653
	    	//diffuse.b *= 0.8*1.0+ uv1.x *0.3076; //0.65 //0.397
		} else {
		    //diffuse.r *= 1.5*1.0;
		    diffuse.g =1.0;//*= 1.4*1.0;
		   // diffuse.b *= 1.1*1.0;
		}
	#endif
	
	#ifdef DYNAMIC_SHADOWS
		vec4 cf = color;
		if(cf.r < 0.655*sin(TIME/125.0-10.0)+0.76 && cf.g < 0.655*sin(TIME/125.0-10.0)+0.76 && cf.b < 0.655*sin(TIME/125.0-10.0)+0.76){//84-30
		    diffuse.r *= 0.6+ uv1.x *0.7857-0.1;//0.64
		    diffuse.g *= 0.6+ uv1.x *0.6428-0.1;//0.613
	    	diffuse.b *= 0.9+ uv1.x *0.0576-0.2;//0.13
		}
	#endif
	
	#ifdef FOG
		diffuse.rgb = mix( diffuse.rgb, fogColor.rgb, fogColor.a );
	#endif
	
	
	gl_FragColor = diffuse;

	#endif // BYPASS_PIXEL_SHADER
}
