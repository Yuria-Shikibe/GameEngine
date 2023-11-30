#version 330 core
layout (location = 0) attribute vec4 aPos;
void main()
{
	gl_Position = aPos;
}