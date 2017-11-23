// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.


//Конфигурация
#define LIGHTS //Освещение TWS
#define FOG_MODE //Туман TWS
//#define NIGHT_VISION //Ночное зрение

//Тестирование
//#define TEST_TEXTURE_MAPS // Классная графика. Вкл - лайтмэп Выкл - шадоумэп
//#define TEST //Для тестирования
//#define UsernameAKs_Lights //Оранжевое освещение от Синкремента
#define UsernameAKs_Water //Вода от Синкремента
//#define COLOR_FILTER //Цветовой фильтр
//#define TORCH_BLINK //Дрожание света от факела
//#define SHADOWS //Статические тени из шадоумэпа
//#define DYNAMIC_SHADOWS //Динамические тени
//#define TORCH_PROCESSOR //Модификация факелов
//#define VERTEX_ONLY //Пропустить фрагментальный шейдер
#define WATER_LIGHT_REFLECTION //Отражение света в воде
#define PLAYER_SHADOW

#include "shaders/fragmentVersionCentroid.h"

#if __VERSION__ >= 300
	#ifndef BYPASS_PIXEL_SHADER
		#if defined(TEXEL_AA) && defined(TEXEL_AA_FEATURE) //антиалясинг
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

varying vec3 fogPos;

#ifdef FOG_MODE
	varying float fog_val; //Количество тумана
	varying float camDis; //Дистанция тумана
#endif


#ifdef PLAYER_SHADOW
	varying float player_shadow;
#endif

#ifdef FOG
	varying vec4 fogColor;
#endif

#ifdef NEAR_WATER
	varying float cameraDist;
#endif

#include "shaders/uniformShaderConstants.h"
#include "shaders/util.h"

uniform sampler2D TEXTURE_0; //Textures
uniform sampler2D TEXTURE_1; //Lightmap
uniform sampler2D TEXTURE_2; //Colormap
uniform sampler2D TEXTURE_3; //Perlin

float sigmoid(float x){
	return 1.0 / ( pow(2.7182, -5.0 * x) + 1.0);
}

float porog(float x, float point){
	if(x >= point){
		return 1.0;
	}else{
		return 0.0;
	}
}

void main(){
	//Заглушка
	#ifdef BYPASS_PIXEL_SHADER
		gl_FragColor = vec4(0, 0, 0, 0);
		return;
	#else

	vec4 diffuse;
	vec2 _tmp = uv1;
	#ifdef PLAYER_SHADOW
		_tmp.y -= player_shadow;
	#endif
	vec4 lights = texture2D( TEXTURE_1, _tmp );//vec2(uv1.x, uv1.y - pshadow) );

	#if !defined(TEXEL_AA) || !defined(TEXEL_AA_FEATURE)
		diffuse = texture2D( TEXTURE_0, uv0 ); //Антиалясинг
	#else
		diffuse = texture2D_AA(TEXTURE_0, uv0 );
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
	
	#ifdef TORCH_PROCESS
		diffuse = TorchProcessor(diffuse, lights);
	#endif
	
	
	#ifdef UsernameAKs_Lights	
		lights.rgb *= inColor.rgb*0.75;
		lights.r *= 0.5;
		lights.g *= 0.3;
		lights.b *= 0.0;
//		lights.rgb *= 1.5;
		diffuse *= inColor*lights;
	#endif
	
	#ifdef LIGHTS
		#ifndef NEAR_WATER
		if(lights.b > lights.r && lights.b > lights.g && lights.r < .5){ //Night
		//FIXME: режет
			/*lights.r *= 1.;
			lights.g *= .75;
			lights.b *= .01;*/
			lights.b *= 1.18;
			lights.r *= 1.16;
			lights.g *= 1.17;
		}/*else if(lights.b == lights.r){ //Noon
			lights.r *= 1.;
			lights.g *= .9;
			lights.b *= .5;
		}*/else{ //light sources
			lights.r *= 1.;
			lights.g *= .8;
			lights.b *= .7;
			lights.rgb *= 1.3;
		}
		#endif
	#endif
	
	#ifdef NEAR_WATER
		#ifdef WATER_LIGHT_REFLECTION
			//TODO: Синкремент, сюда пиши свой код
			//Работай с lights
		#endif
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

	#ifdef SHADOWS
		/*if(diffuse.r > 2.1 && diffuse.g > 2.1 && diffuse.b > 2.1) {
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
		}*/
		diffuse.rgb *= -vec3(texture2D( TEXTURE_2, inColor.xy).g) + 1.5;
	#endif
	
	#ifdef DYNAMIC_SHADOWS
		vec4 cf = color;
		if(cf.r < (0.655 * sin( TIME / 125.0 - 10.0 ) + 0.76)
			&& cf.g < (0.655 * sin( TIME / 125.0 - 10.0 ) + 0.76)
			&& cf.b < (0.655 * sin( TIME / 125.0 - 10.0 ) + 0.76)
		){//84-30
		    diffuse.r *= 0.6+ uv1.x *0.7857-0.1;//0.64
		    diffuse.g *= 0.6+ uv1.x *0.6428-0.1;//0.613
	    	diffuse.b *= 0.9+ uv1.x *0.0576-0.2;//0.13
		}
	#endif
	
	#ifdef FOG
		diffuse.rgb = mix( diffuse.rgb, fogColor.rgb, fogColor.a );
	#endif
	
	#ifdef FOG_MODE
		//TODO: .7 заменить на переменную времени дня
		vec3 fog_color = mix(vec3(0.8, 0.9, 1.0) * (0.3 + .7 * 0.6), vec3(0.9, 0.6, 0.7), (1.0 - abs(0.5 - .7) * 2.0) * 0.7);
		float fog_distance = min(1.0, camDis / 24.0 - 0.2);
		diffuse.rgb = mix(diffuse.rgb, fog_color, fog_distance * max(0.5, fog_val));
	#endif
	
	#ifdef TEST
		diffuse = texture2D( TEXTURE_0, uv0 / 32.0); //Меняем текстуру воды, что бы было лучше (на самом деле хуже) видно
	#endif
	
	#ifdef TEST_TEXTURE_MAPS
		#ifdef FANCY //Включена Классная графика в настройках
			diffuse = lights;//texture2D( TEXTURE_1, uv1 ); //Light map
		#else
			diffuse = texture2D( TEXTURE_2, inColor.xy); //Shadow map
		#endif
	#endif

	#ifdef NEAR_WATER
		#ifdef UsernameAKs_Water
			//diffuse += (cnoise(fogPos) + 1.0) / 16.0;
			diffuse += texture2D(TEXTURE_3, fract(fogPos.xz / 16.0)).r / 8.0;
		#endif
	#endif

	#ifdef VERTEX_ONLY
		diffuse = color;
	#endif
	
	gl_FragColor = diffuse;
	//gl_FragColor = vec4(uv0 / 32.0, 0.0, 1.0); 

	#endif // BYPASS_PIXEL_SHADER
}
