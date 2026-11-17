#version 330 core

layout(location = 0) in vec2 aCorner;

uniform mat4 uProj;
uniform mat4 uModel;
out vec2 vUV;

void main(){
    vUV = aCorner;
    gl_Position = uProj * uModel * vec4(aCorner,0.0, 1.0);
}