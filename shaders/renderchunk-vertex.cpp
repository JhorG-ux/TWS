// __multiversion__
// This signals the loading code to prepend either #version 100 or #version 300 es as apropriate.

//On/Off
#define UNDER_WATER
#define WATER_WAVES
#define PLANT_WAVES

#define PlantWavesSpeed 0.783
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

attribute POS4 POSITION;
attribute vec4 COLOR;
attribute vec2 TEXCOORD_0;
attribute vec2 TEXCOORD_1;

const float rA = 1.0;
const float rB = 1.0;
const vec3 UNIT_Y = vec3(0,1,0);
const float DIST_DESATURATION = 56.0 / 255.0;
//WARNING this value is also hardcoded in the water color, don'tchange

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
			if(color.g > color.b){ // Если они зеленые
				POS3 l = POSITION.xyz;
				pos.s += sin(TIME * PlantWavesSpeed + 3.0*l.x+l.y+l.z) * 0.05; // Применить искажение
			}
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
			vec3 waves = sin(POSITION.xyz * 10.0 + TIME * UnderWaterSpeed)  * cos(POSITION.y * 10.0 + TIME * UnderWaterSpeed) * range;  //Скорость: 6.0(или в конфиге)
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
	gl_Position = pos;
}
