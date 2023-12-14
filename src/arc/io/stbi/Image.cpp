import Image;

import <memory>;
import <GLFW/glfw3.h>;
import Graphic.Pixmap;

using std::shared_ptr;
using namespace stbi;

std::shared_ptr<GLFWimage> obtain_GLFWimage(const Graphic::Pixmap& pixmap) {
	return std::make_shared<GLFWimage>(pixmap.getWidth(), pixmap.getHeight(), pixmap.copyData().release());
}
