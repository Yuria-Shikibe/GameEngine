import Core;

import <functional>;
import <glad/glad.h>;
import <GLFW/glfw3.h>;

import Graphic;
import Geom.Shape.Rect_Orthogonal;

import Core.Audio;
import Assets.Manager;
import Assets.Bundle;
import Core.Settings;
import Core.Log;

import UI.Root;

import RuntimeException;

import Core.Batch.Batch_Sprite;
import Core.Batch;
import Core.Input;
import Core.Camera;
import Core.Renderer;
import GL;
import GL.Constants;
import OS.FileTree;
import OS.Key;
import OS;
import <iostream>;

void Core::setScreenBound(GLFWwindow* win) {
	glfwGetWindowSize(win, lastScreenBound.getWidthRaw(), lastScreenBound.getHeightRaw());
	glfwGetWindowPos(win, lastScreenBound.getSrcXRaw(), lastScreenBound.getSrcYRaw());
}
