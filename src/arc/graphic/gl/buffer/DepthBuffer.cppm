module;

#include <glad/glad.h>

export module GL.DepthBuffer;

export import GL.Object;
import Graphic.Resizeable;
import GL.Texture;

export namespace GL{
	class DepthBuffer : public GL::Texture{
	public:
		explicit DepthBuffer(const GLsizei w = 2, const GLsizei h = 2) : Texture(GL_TEXTURE_2D, w, h){
			glCreateTextures(targetFlag, 1, &nameID);
			glTextureStorage2D(nameID, 1, GL_DEPTH_COMPONENT, width, height);

			setParameters();
		}
	};
}
