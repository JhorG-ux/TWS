// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.


//Конфигурация
//#define NIGHT_VISION //Ночное зрение
//#define COLOR_FILTER //Цветовой фильтр
//#define TORCH_BLINK //Дрожание света от факела

//Тестирование
//#define TEST_TEXTURE_MAPS // Классная графика. Вкл - лайтмэп Выкл - шадоумэп
//#define TEST_UV
//#define UsernameAKs_Lights
//#define SHADOWS //Статические тени из шадоумэпа
//#define DYNAMIC_SHADOWS //Динамические тени
//#define TORCH_PROCESSOR //Модификация факелов
//#define VERTEX_ONLY

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

varying vec3 fogPos;

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
uniform sampler2D TEXTURE_2; //Shadowmap

//	Classic Perlin 3D Noise
//	by Stefan Gustavson
//
vec4 permute(vec4 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
vec3 fade(vec3 t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

float cnoise(vec3 P)
{
	vec3 Pi0 = floor(P);		// Integer part for indexing
	vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
	Pi0 = mod(Pi0, 289.0);
	Pi1 = mod(Pi1, 289.0);
	vec3 Pf0 = fract(P);		// Fractional part for interpolation
	vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
	vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
	vec4 iy = vec4(Pi0.yy, Pi1.yy);
	vec4 iz0 = Pi0.zzzz;
	vec4 iz1 = Pi1.zzzz;

	vec4 ixy = permute(permute(ix) + iy);
	vec4 ixy0 = permute(ixy + iz0);
	vec4 ixy1 = permute(ixy + iz1);

	vec4 gx0 = ixy0 / 7.0;
	vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
	gx0 = fract(gx0);
	vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
	vec4 sz0 = step(gz0, vec4(0.0));
	gx0 -= sz0 * (step(0.0, gx0) - 0.5);
	gy0 -= sz0 * (step(0.0, gy0) - 0.5);

	vec4 gx1 = ixy1 / 7.0;
	vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
	gx1 = fract(gx1);
	vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
	vec4 sz1 = step(gz1, vec4(0.0));
	gx1 -= sz1 * (step(0.0, gx1) - 0.5);
	gy1 -= sz1 * (step(0.0, gy1) - 0.5);

	vec3 g000 = vec3(gx0.x, gy0.x, gz0.x);
	vec3 g100 = vec3(gx0.y, gy0.y, gz0.y);
	vec3 g010 = vec3(gx0.z, gy0.z, gz0.z);
	vec3 g110 = vec3(gx0.w, gy0.w, gz0.w);
	vec3 g001 = vec3(gx1.x, gy1.x, gz1.x);
	vec3 g101 = vec3(gx1.y, gy1.y, gz1.y);
	vec3 g011 = vec3(gx1.z, gy1.z, gz1.z);
	vec3 g111 = vec3(gx1.w, gy1.w, gz1.w);

	vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
	g000 *= norm0.x;
	g010 *= norm0.y;
	g100 *= norm0.z;
	g110 *= norm0.w;
	vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
	g001 *= norm1.x;
	g011 *= norm1.y;
	g101 *= norm1.z;
	g111 *= norm1.w;

	float n000 = dot(g000, Pf0);
	float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
	float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
	float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
	float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
	float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
	float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
	float n111 = dot(g111, Pf1);

	vec3 fade_xyz = fade(Pf0);
	vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
	vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
	float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
	return 2.2 * n_xyz;
}

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

	vec4 diffuse;
	vec4 lights = texture2D( TEXTURE_1, uv1 );

	#if !defined(TEXEL_AA) || !defined(TEXEL_AA_FEATURE)
		diffuse = texture2D( TEXTURE_0, uv0 );
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
		lights.rgb *= inColor.rgb*0.75;
		lights.r *= 0.5;
		lights.g *= 0.3;
		lights.b *= 0.0;
//		lights.rgb *= 1.5;
		diffuse *= inColor*lights;
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
		diffuse.rgb *= -vec3(texture2D( TEXTURE_2, inColor.xy).g) + 1.0;
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
	
	#ifdef TEST_UV
/*		diffuse.r = uv1.x;
		diffuse.b = uv1.y;
		diffuse.g = 0.0;
		diffuse.a = 1.0;*/
		//vec4 lightmap = texture2D( TEXTURE_1, uv1);
		//float tmplmmultipler = pow(tmplightmap.r, 2)
		//diffuse = tmplightmap * pow(tmplightmap.r, 2.0);//texture2D( TEXTURE_0, uv0) * tmplightmap * vec4(tmplmmultipler,1.0);
		//diffuse = lightmap * ( 1.0 / ( pow( 2.7182, -5.0 * lightmap.r ) + 1.0 ) );
		//diffuse.rgb = vec3(lightmap.r);//lightmap * porog(lightmap.r, 0.9);
		diffuse = inColor;
	#endif
	
	#ifdef TEST_TEXTURE_MAPS
		#ifdef FANCY //Включена Классная графика в настройках
			diffuse = texture2D( TEXTURE_1, uv1 ); //Light map
		#else
			diffuse = texture2D( TEXTURE_2, inColor.xy); //Shadow map
		#endif
	#endif

	#ifdef NEAR_WATER
		diffuse += (cnoise(fogPos) + 1.0) / 16.0;
	#endif

	#ifdef VERTEX_ONLY
		diffuse = color;
	#endif
	
	gl_FragColor = diffuse;

	#endif // BYPASS_PIXEL_SHADER
}
