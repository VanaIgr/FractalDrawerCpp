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
	vec2 pivotPoints[];
};

uniform vec2 viewPortPivot1, viewPortPivot2;

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

	return apply;
}

mat2 toMat(vec2 p1, vec2 p2) {
	return inverse(mat(p1, p2));
}

void main(void) {
	vec2 coord_raw = vec2(gl_FragCoord.x, windowSize.y - gl_FragCoord.y);
	color = texture2D(canvas, coord_raw / windowSize.xy);

	if (distance(pivotPoints[0], coord_raw) < 3) color = vec4(1, 1, 0, 1);
	if (distance(pivotPoints[1], coord_raw) < 3) color = vec4(1, 0, 1, 1);

	for (uint i = fractalPivotsStart; i < pivotPoints.length(); i++) {
		if (distance(pivotPoints[i], coord_raw) < 3) color = vec4(0, i % 2, (i+1) % 2, 1);
	}

	for (uint i = 0; i < pivotPoints.length(); i+= 2) {
		vec2 p1 = pivotPoints[i];
		vec2 p2 = pivotPoints[i + 1];
		vec2 cc = toMat(p1, p2) * (coord_raw - p1);
		if (clamp(cc, vec2(0, 0), vec2(1, 1)) != cc)
			color = mix(color, vec4(0.5, 0.5, 0.5, 1), 0.1);
	}

}