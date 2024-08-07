module;

#include <glad/glad.h>

export module GL.Texture;

export import GL.Object;
export import Graphic.Resizeable;
import std;

export namespace GL{
	enum TexParams{
		nearest                = GL_NEAREST,
		linear                 = GL_LINEAR,
		mipmap_linear_linear   = GL_LINEAR_MIPMAP_LINEAR,
		mipmap_nearest_nearest = GL_NEAREST_MIPMAP_NEAREST,
		mipmap_nearest_linear  = GL_NEAREST_MIPMAP_LINEAR,
		mipmap_linear_nearest  = GL_LINEAR_MIPMAP_NEAREST,
		filterMag              = GL_TEXTURE_MAG_FILTER,
		filterMin              = GL_TEXTURE_MIN_FILTER,
		wrap_U                 = GL_TEXTURE_WRAP_S,
		wrap_V                 = GL_TEXTURE_WRAP_T,
		clamp_to_edge          = GL_CLAMP_TO_EDGE,
		//...
	};

	struct Texture : GLObject, Graphic::ResizeableInt{
	protected:
		int width = 0, height = 0;

	public:
		[[nodiscard]] int getWidth() const {
			return width;
		}

		[[nodiscard]] int getHeight() const {
			return height;
		}

		Texture() = default;

		explicit Texture(const GLenum targetFlag)
			: GLObject(targetFlag){
		}

		Texture(const GLenum targetFlag, const int width, const int height)
			: GLObject(targetFlag),
			  width(width),
			  height(height){
		}

		void setParameters(const GLint downScale = GL::mipmap_linear_linear, const GLint upScale = GL::linear, const GLint clampX = GL_CLAMP_TO_EDGE, const GLint clampY = GL_CLAMP_TO_EDGE) const{
			glTextureParameteri(nameID, GL_TEXTURE_MIN_FILTER, downScale);
			glTextureParameteri(nameID, GL_TEXTURE_MAG_FILTER, upScale);
			glTextureParameteri(nameID, GL_TEXTURE_WRAP_S, clampX);
			glTextureParameteri(nameID, GL_TEXTURE_WRAP_T, clampY);
		}

		void setFilter(const GLint downScale = GL::mipmap_nearest_linear, const GLint upScale = GL::linear) const{
			glTextureParameteri(nameID, GL_TEXTURE_MIN_FILTER, downScale);
			glTextureParameteri(nameID, GL_TEXTURE_MAG_FILTER, upScale);
		}

		void setWrap(const GLint clamp = GL::clamp_to_edge) const{
			glTextureParameteri(nameID, GL_TEXTURE_WRAP_S, clamp);
			glTextureParameteri(nameID, GL_TEXTURE_WRAP_T, clamp);
		}

		void unbind() const{
			glBindTexture(targetFlag, 0);
		}

		void bind() const{
			glBindTexture(targetFlag, nameID);
		}

		~Texture() override = default;

		Texture(const Texture& other) = delete;

		Texture(Texture&& other) noexcept = default;

		Texture& operator=(const Texture& other) = delete;

		Texture& operator=(Texture&& other) noexcept = default;

		virtual void active(unsigned offset) const = 0;
		virtual void activeAll(unsigned offset) const = 0;
		void resize(int w, int h) override = 0;
	};
}
