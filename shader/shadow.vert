layout(location = 0) in vec3 aPos;

uniform mat4 light_space;

void main() {
    gl_Position = light_space * vec4(aPos, 1);
}
