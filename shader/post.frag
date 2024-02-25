in vec2 uv;

out vec4 out_Color;

uniform sampler2D color;

void main(void) {
    vec3 final_color = texture(color, uv).rgb;

    // float gamma = 1.3;
    float gamma = 2.2;
    final_color = pow(final_color, vec3(1.0 / gamma));
    out_Color = vec4(final_color, 1.0);
}
