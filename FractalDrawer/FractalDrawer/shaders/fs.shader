#version 430

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform ivec2 windowSize;

uniform sampler2D canvas_prev;

in vec4 gl_FragCoord;
out vec4 color;

struct PaintSample {
    uint posInChain; // 0 - start, 1 - interm, 2 - end
    bool isDelete;
    vec2 pos;
};

layout(std430, binding = 1) buffer PaintSamples {
    uint samplesCount;
    PaintSample samples[];
};

void main(void) {
    vec2 coord = gl_FragCoord.xy;

    vec4 color_prev = texture2D(canvas_prev, gl_FragCoord.xy / windowSize.xy);

    color = color_prev;
    for (uint i = 0; i < samplesCount; i++) {
        vec4 color_new;
        if (samples[i].isDelete) color_new = vec4(0, 0, 0, 1);
        else color_new = vec4(1, 0, 0, 1);

        color = mix(color, color_new, distance(coord, vec2(samples[i].pos.x, samples[i].pos.y)) < 40);

    }


}