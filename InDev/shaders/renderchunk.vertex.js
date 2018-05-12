#version 100

#define WAVS
#define UNDER_WATER

uniform MAT4 WORLDVIEWPROJ;
uniform vec4 FOG_COLOR;
uniform vec2 FOG_CONTROL;
uniform float RENDER_DISTANCE;
uniform vec2 VIEWPORT_DIMENSION;
uniform vec4 CURRENT_COLOR;		//current color r contains the offset to apply to the fog for the "fade in"
uniform POS3 CHUNK_ORIGIN;
uniform POS3 VIEW_POS;
uniform float FAR_CHUNKS_DISTANCE;
uniform highp float TIME;

attribute POS4 POSITION;
attribute vec4 COLOR;
attribute vec2 TEXCOORD_0;
attribute vec2 TEXCOORD_1;

varying vec2 uv0;
varying vec2 uv1;
varying vec4 color;

#ifdef FOG
	varying vec4 fogColor;
#endif

const float rA = 1.0;
const float rB = 1.0;
const vec3 UNIT_Y = vec3(0,1,0);
const mediump float maxDist = 0.9579790980632242;
const highp float pi = 3.1415926535898932; //значение числа π =D

const float DIST_DESATURATION = 56.0 / 255.0; //WARNING this value is also hardcoded in the water color, don'tchange

void main()
{

		//Переменные для работы
	
	//Позиция
	vec3 xyz = POSITION.xyz + CHUNK_ORIGIN;
	vec4 pos = POSITION;

  //Задаём значения...
		uv0 = TEXCOORD_0;
		uv1 = TEXCOORD_1;
		color = COLOR;


// Ищем дистанцию до камеры

			vec3 relPos = VIEW_POS - (POSITION.xyz + CHUNK_ORIGIN);
			float cameraDepth = length(relPos);

// Туманчик)

#ifdef FOG
	float len = cameraDepth / RENDER_DISTANCE;
	#ifdef ALLOW_FADE
		len += CURRENT_COLOR.r;
	#endif

	fogColor.rgb = FOG_COLOR.rgb;
	fogColor.a = clamp((len - FOG_CONTROL.x) / (FOG_CONTROL.y - FOG_CONTROL.x), 0.0, 1.0);
	#ifdef UNDER_WATER
	//Туман под водой
	if(FOG_CONTROL.x < 0.1 && FOG_CONTROL.y < 20.0){
		float range = 0.035 + cameraDepth / 1000.0; //Больше реалистичности!
		//float range = 0.035; //Меньше реалистичности)
		vec3 sinedata = xyz * 20.0 + TIME * 1.5;  //Скорость: 6.0(или в конфиге)
		pos.x += sin(sinedata.z) * cos(sinedata.z) * range; //Искажение по X
		pos.y += sin(sinedata.x) * cos(sinedata.x) * range; //Искажение по Y (верт)
		pos.z += sin(sinedata.y) * cos(sinedata.y) * range; //Искажение по Z
		fogColor.a *= 0.5; //0.8 //Добавляем прозрачности =)
  }
  #endif
#endif

///// water magic
#ifdef NEAR_WATER // Если в воде
			
	vec4 tpos = POSITION; // Рабочая позиция
	float wav = 0.0;
	float h = fract(tpos.y); //Глубина
	
	float c0 = max(sign(h - 0.8125), 0.0);
	float c1 = 1.0 - max(sign(h - 0.9), 0.0);
	#ifdef WAVS
				//Добавляем волны!
				wav = (cos(TIME * 1.5 + xyz.x) /2.0 * sin(TIME * 1.5 + xyz.z) / 2.0)-0.5;
				tpos.y += wav * c0 * c1;
				pos += tpos;
	#endif
	#ifdef FANCY  /////enhance water //И включена графика
		float F = dot(normalize(relPos), UNIT_Y);
		F = 1.0 - max(F, 0.1);
		F = 1.0 - mix(F*F*F*F, 1.0, min(1.0, cameraDepth / FAR_CHUNKS_DISTANCE));

		color.rg -= vec2(F * DIST_DESATURATION);

		vec4 depthColor = vec4(color.rgb * 0.5, 1.0);
		vec4 traspColor = vec4(color.rgb * 0.45, 0.8);
		vec4 surfColor = vec4(color.rgb, 1.0);

		vec4 nearColor = mix(traspColor, depthColor, color.a);
		color = mix(surfColor, nearColor, F);
	#else //И выключена графика
	
		// Для Nexus 6
		vec4 surfColor = vec4(color.rgb, 1.0);
		color = surfColor;
		color.a = pos.z / FAR_CHUNKS_DISTANCE + 0.5;
	#endif //FANCY
#endif

//Вывод
gl_Position = WORLDVIEWPROJ * pos;
}