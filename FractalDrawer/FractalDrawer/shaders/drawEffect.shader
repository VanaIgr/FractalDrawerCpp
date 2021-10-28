#version 430

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform ivec2 windowSize;

uniform sampler2D canvas;

in vec4 gl_FragCoord;
out vec4 color;

const uint fractalPivotsStart = 2;
layout(std430, binding = 2) buffer Pivots {
	vec2 pivotPoints[]; // 2 viewport pivots, the rest are fractal pivots
};

mat2 mat(vec2 p1, vec2 p2) {
	float a = atan(windowSize.y, windowSize.x);

	mat2 rot = mat2(
		cos(a), -sin(a),
		sin(a), cos(a)
	);

	float b = radians(180) / 2;
	mat2 rot90 = mat2(
		cos(b), -sin(b),
		sin(b), cos(b)
	);

	vec2 d = p2 - p1;

	vec2 h = rot * d;
	vec2 v = rot90 * h;

	h *= cos(-a);
	v *= sin(-a);

	mat2 apply = mat2(
		h.x, h.y,
		v.x, v.y
	);

	return (apply);
}

mat2 toMat(vec2 p1, vec2 p2) {
	float a = atan(windowSize.y, windowSize.x);

	mat2 rot = mat2(
		cos(a), -sin(a),
		sin(a), cos(a)
	);

	float b = radians(180) / 2;
	mat2 rot90 = mat2(
		cos(b), -sin(b),
		sin(b), cos(b)
	);

	vec2 d = p2 - p1;

	vec2 h = rot * d;
	vec2 v = rot90 * h;

	h *= cos(-a);
	v *= sin(-a);

	mat2 apply = mat2(
		h.x, h.y,
		v.x, v.y
	);

	return inverse(apply);
}

vec3 rgb2hsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(void) {
	vec2 coord_raw = vec2(gl_FragCoord.x, windowSize.y - gl_FragCoord.y);
	//vec2 fractalCoord = toMat(pivotPoints[0], pivotPoints[1]) * ((gl_FragCoord.xy - pivotPoints[0])) * windowSize.xy;
	//fractalCoord.y = windowSize.y - fractalCoord.y;

	const uint pivotCount = pivotPoints.length();
	const uint fractalS = (pivotCount - 2) / 2;
	color = vec4(0, 0, 0, 1);
	for (uint i = 2; i < pivotCount; i+= 2) {
		vec2 p1 = pivotPoints[i];
		vec2 p2 = pivotPoints[i + 1];
		vec2 cc = toMat(p1, p2) * (gl_FragCoord.xy - p1);
		if (clamp(cc, vec2(0, 0), vec2(1, 1)) == cc) {
			vec2 fractalCoord = mat(pivotPoints[0], pivotPoints[1]) * cc.xy + pivotPoints[0];
			fractalCoord /= windowSize.xy;
			if (clamp(fractalCoord, vec2(0, 0), vec2(1, 1)) == fractalCoord) {
				color += vec4(
					hsv2rgb(
						vec3(
							(float(i)/2.0 / float(fractalS)), 
							1.0,
							rgb2hsv(texture2D(canvas, fractalCoord).xyz).z
						)
					), 1.0);
			}
		}
	}

	color.a = 1;
}