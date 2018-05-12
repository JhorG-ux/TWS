#version 100
/*
###################################
*********TeamWorldShaders**********
========== by PROPHESSOR ==========
~~~~~~~~~Все права защищены!~~~~~~~
###################################
*/

#define DYNAMIC_SHADOWS
// #define SUN_FOG

uniform sampler2D TEXTURE_0;
uniform sampler2D TEXTURE_1;
uniform sampler2D TEXTURE_2;
//uniform vec4 FOG_COLOR;
//uniform vec4 CURRENT_COLOR;
uniform highp float TIME;

//Переменные

varying vec2 uv0;
varying vec2 uv1;
varying vec4 color;
//varying float time;		//Время
//varying vec4 skyColor;	//Цвет неба

#ifdef FOG
	varying vec4 fogColor;
#endif

//Константы

void main()
{
	
	// Временные переменные для работы
	
	vec4 tex = texture2D( TEXTURE_0, uv0 ); //Текстура, координаты
	vec4 inColor = color;	
	vec4 diffuse = tex;
	
	//Конец временных переменных
	
#ifdef SEASONS_FAR
	diffuse.a = 1.0;
	inColor.b = 1.0;
#endif

#ifdef ALPHA_TEST
	if(diffuse.a < 0.5)
	 	discard;
#endif
	
	diffuse *= texture2D( TEXTURE_1, uv1 );

#ifndef SEASONS


	//Настройка освещения
	vec3 lightf;
	lightf = vec3(0.1, 0.0, 0.0);
	diffuse.rgb += lightf;

#ifdef SUN_FOG	
	vec4 sun;
	sun = vec4(0.8, 0.4, 0.1, 0.5) * pow(uv1.x * 1.0, 1.0);
	diffuse += sun;
#endif

#if !defined(ALPHA_TEST) && !defined(BLEND)
	diffuse.a = inColor.a;
#elif defined(BLEND)
	diffuse.a *= inColor.a;
#endif	
	
	diffuse.rgb *= inColor.rgb;
#else
	vec2 uv = inColor.xy;
	uv.y += 1.0 / 512.0;
	diffuse.rgb *= mix(vec3(1.0,1.0,1.0), texture2D( TEXTURE_2, uv).rgb*2.0, inColor.b);
	diffuse.rgb *= inColor.aaa;
	diffuse.a = 1.0;
#endif

#ifdef FOG //fancy+fog
	//Правка тумана
	// fogColor.rgb += vec3();
	diffuse.rgb = mix( diffuse.rgb, fogColor.rgb, fogColor.a );
#endif


	gl_FragColor = diffuse; //Вывод
//Правки gl_FragColor
	//Тени
	#ifdef OLD_DYNAMIC_SHADOWS
		vec4 TWSshadows = color;
//	  if(TWSshadows.r < 0.655*sin(TIME/60.0-10.0)+0.76 && TWSshadows.g < 0.655*sin(TIME/60.0-10.0)+0.76 && TWSshadows.b < 0.655*sin(TIME/60.0-10.0)+0.76){ //125
	    gl_FragColor.r *= 0.6 + uv1.x *1.7857-0.1;
	    gl_FragColor.g *= 0.6 + uv1.x *1.6428-0.1;
	    gl_FragColor.b *= 0.9 + uv1.x *1.0576-0.2;
// }
	#endif
	#ifdef DYNAMIC_SHADOWS
	
	#endif
}
