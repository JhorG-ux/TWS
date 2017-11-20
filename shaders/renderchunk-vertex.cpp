// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

//On/Off
#define UNDER_WATER
#define WATER_WAVES
#define PLANT_WAVES
//#define ADVANCED_PLANT_FILTER

//Тестирование
//#define TEST

#define PlantWavesSpeed 0.9 //0.783
#define WaterWavesSpeed 2.0
#define UnderWaterSpeed 1.0//1.5

#include "shaders/vertexVersionCentroid.h"
#if __VERSION__ >= 300
	#ifndef BYPASS_PIXEL_SHADER
		_centroid out vec2 uv0;
		_centroid out vec2 uv1;
	#endif
#else
	#ifndef BYPASS_PIXEL_SHADER
		varying vec2 uv0;
		varying vec2 uv1;
	#endif
#endif

#ifndef BYPASS_PIXEL_SHADER
	varying vec4 color;
#endif

#ifdef FOG
	varying vec4 fogColor;
#endif

#ifdef NEAR_WATER
	varying float cameraDist;
#endif

varying vec3 fogPos;

#include "shaders/uniformWorldConstants.h"
#include "shaders/uniformPerFrameConstants.h"
#include "shaders/uniformShaderConstants.h"
#include "shaders/uniformRenderChunkConstants.h"

#ifdef ADVANCED_PLANT_FILTER
	uniform sampler2D TEXTURE_0; //Textures
#endif

attribute POS4 POSITION;
attribute vec4 COLOR;
attribute vec2 TEXCOORD_0;
attribute vec2 TEXCOORD_1;

const float rA = 1.0;
const float rB = 1.0;
const vec3 UNIT_Y = vec3(0,1,0);
const float DIST_DESATURATION = 56.0 / 255.0;
//WARNING this value is also hardcoded in the water color, don'tchange


//random
/*
float rnd(float x, float z, float seed){
	float v = mod(x * 1372.0 + z * 1227.0 + seed * 1293.0, 10000.0);
	v = mod(mod(v * 1526.0 + 6285.0, 10000.0) * 5336.0 + 5637.0, 10000.0);
	if(mod(v, 2.0) == 1.0){
		v = mod(v * 5383.0 + 5732.0, 10000.0);
	}
	if(mod(v, 2.0) == 0.0){
		v = mod(v * 5763.0 + 3232.0, 10000.0);
	}
	return v / 10000.0;
}
*/
/*
float noisegen(float x){
	return fract(sin(x)*0.8+0.5);
}
*/
void main() {
	POS4 worldPos;
	#ifdef AS_ENTITY_RENDERER
		vec4 fogPos4 = WORLD * POSITION;
		POS4 pos = WORLDVIEWPROJ * POSITION;
		worldPos = pos;
	#else
		vec4 fogPos4;
		worldPos.xyz = (POSITION.xyz * CHUNK_ORIGIN_AND_SCALE.w) + CHUNK_ORIGIN_AND_SCALE.xyz;
		worldPos.w = 1.0;

		// Transform to view space before projection instead of all at once to avoid floating point errors
		// Not required for entities because they are already offset by camera translation before rendering
		// World position here is calculated above and can get huge
		POS4 pos = WORLDVIEW * worldPos;
		pos = PROJ * pos;
		fogPos4 = worldPos;
	#endif

	fogPos = fogPos4.xyz / fogPos4.w;

	#ifndef BYPASS_PIXEL_SHADER
		uv0 = TEXCOORD_0;
		uv1 = TEXCOORD_1;
		color = COLOR;
	#endif

	///// find distance from the camera

	#if defined(FOG) || defined(NEAR_WATER)
		#ifdef FANCY
			vec3 relPos = -worldPos.xyz;
			float cameraDepth = length(relPos);
			#ifdef NEAR_WATER
				cameraDist = cameraDepth / FAR_CHUNKS_DISTANCE;
			#endif
		#else
			float cameraDepth = pos.z;
			#ifdef NEAR_WATER
				vec3 relPos = -worldPos.xyz;
				float camDist = length(relPos);
				cameraDist = camDist / FAR_CHUNKS_DISTANCE;
			#endif
		#endif
	#endif

	// На прозрачных штуках
	#ifdef ALPHA_TEST
		#ifdef PLANT_WAVES
			// if(color.g > color.b){ // Если они зеленые
			#ifdef ADVANCED_PLANT_FILTER
				/*vec4 texcolor = texture2D( TEXTURE_0, uv0 );
				float r = texcolor.r,
					  g = texcolor.g,
					  b = texcolor.b;*/
				//if(){ //Определение колыхающихся блоков на основе текстур
				vec2 texcoord = vec2(uv0.x * 32.0, uv0.y * 16.0);
   				float index = floor(texcoord.x) + floor(texcoord.y) * 32.0;
				float power = 0.0;
				if(index >= 151.0 && index <= 169.0){
					//Спавнер
					//Красный цветок
					//Семена арбуза
 					power = 0.9;
				}else if(index == 180.0 || index == 181.0){
					//Ничего
					power = 0.75;
				}else if(index == 262.0 || index == 263.0){
					//Пшеница 3 и 4 стадии
					power = 0.45;
				}else if(index >= 64.0 && index <= 96.0 || index >= 133.0 && index < 137.0 || index >= 206.0 && index <= 213.0 || index >= 224.0 && index <= 226.0){
					power = 1.0;
					if(texcoord.y > floor(texcoord.y) + .1){
						power = 0.0;
					}
					if(index >= 81.0 && index <= 90.0){
						power++;
					}
				}else if(index >= 361.0 && index <= 362.0){
					power = 0.9;
					if(texcoord.y > floor(texcoord.y) + .1){
						power = 0.0;
					}
				}else if(index == 175.0){
					power = 0.3;
				}
				/*switch(index){
					default:
						float power = 0.05;
						break;
				}*/
			#else
				float power = 0.05;
			#endif
					POS3 l = POSITION.xyz;
					pos.s += sin(TIME * PlantWavesSpeed + 3.0 * l.x + l.y + l.z) * power; // Применить искажение
			#ifdef ADVANCED_PLANT_FILTER
				//}
			#endif
			// }
		#endif
	#endif
	///// apply fog

	#ifdef FOG
		float len = cameraDepth / RENDER_DISTANCE;
		#ifdef ALLOW_FADE
			len += CURRENT_COLOR.r;
		#endif
		
		#ifdef UNDER_WATER
			if(FOG_CONTROL.x < 0.1 && FOG_CONTROL.y < 20.0){
				float range = 0.035;// + cameraDepth / 1000.0;
				vec3 waves = sin(POSITION.xyz * 1.0 + TIME * 1.0) * range;//  * cos(POSITION.y * 10.0 + TIME * UnderWaterSpeed) * range;  //Скорость: 6.0(или в конфиге)
				pos.xyz += waves;
				fogColor.a = 0.5; //0.8 //Мутность воды =)
			}
		#endif

		fogColor.rgb = FOG_COLOR.rgb;
		fogColor.a = clamp((len - FOG_CONTROL.x) / (FOG_CONTROL.y - FOG_CONTROL.x), 0.0, 1.0);
	#endif

	///// water magic
	#ifdef NEAR_WATER
		#ifdef FANCY  /////enhance water
			float F = dot(normalize(relPos), UNIT_Y);
			F = 1.0 - max(F, 0.1);
			// Nvidia Tegra 2 and Xoom (maybe the combination) have some bug where "min" returns a highp float
			// in some cases even though the input are lowp floats. This together with the devices/drivers
			// inability to figure out what to do with that causes it to fail due to an ambiguous call to mix.
			F = 1.0 - mix(F*F*F*F, 1.0, float(min(1.0, cameraDepth / FAR_CHUNKS_DISTANCE)));

			color.rg -= vec2(F * DIST_DESATURATION);

			vec4 depthColor = vec4(color.rgb * 0.5, 1.0);
			vec4 traspColor = vec4(color.rgb * 0.45, 0.8);
			vec4 surfColor = vec4(color.rgb, 1.0);
			
			// Волны на воде
			pos.y += sin(TIME * WaterWavesSpeed + 3.0*fogPos.x+fogPos.y+fogPos.z) * 0.05;

			vec4 nearColor = mix(traspColor, depthColor, color.a);
			color = mix(surfColor, nearColor, F);
		#else
			// Completely insane, but if I don't have these two lines in here, the water doesn't render on a Nexus 6
			vec4 surfColor = vec4(color.rgb, 1.0);
			color = surfColor;
			color.a = pos.z / FAR_CHUNKS_DISTANCE + 0.5;
		#endif //FANCY
	#endif
	
	#ifdef TEST
		#ifndef BYPASS_PIXEL_SHADER
			//color.r = index;
			//color.g = 0.0;
			color.rg = uv0.xy;
			color.b = 0.0;
		#endif
	#endif
	
	gl_Position = pos;
}
