module;

#include <glad/glad.h>

export module GL.Texture;

export import GL.Object;
import <utility>;

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
		//...
	};

	struct Texture : GLObject{
	protected:
		unsigned int width = 0, height = 0;

	public:
		[[nodiscard]] unsigned getWidth() const {
			return width;
		}

		[[nodiscard]] unsigned getHeight() const {
			return height;
		}

		Texture(const Texture& other) = delete;

		Texture(Texture&& other) noexcept
			: GL::GLObject(std::move(other)),
			  width(other.width),
			  height(other.height){
		}

		Texture& operator=(const Texture& other) = delete;

		Texture& operator=(Texture&& other) noexcept{
			if(this == &other) return *this;
			GL::GLObject::operator =(std::move(other));
			width = other.width;
			height = other.height;
			return *this;
		}

		explicit Texture(const GLenum targetFlag)
			: GLObject(targetFlag){
		}

		Texture(const GLenum targetFlag, const unsigned int width, const unsigned int height)
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

		void setScale(const GLint downScale = GL::mipmap_nearest_linear, const GLint upScale = GL::linear) const{
			glTextureParameteri(nameID, GL_TEXTURE_MIN_FILTER, downScale);
			glTextureParameteri(nameID, GL_TEXTURE_MAG_FILTER, upScale);
		}

		void setWrap(const GLint clamp = GL_CLAMP_TO_EDGE) const{
			glTextureParameteri(nameID, GL_TEXTURE_WRAP_S, clamp);
			glTextureParameteri(nameID, GL_TEXTURE_WRAP_T, clamp);
		}

		void unbind() const{
			glBindTexture(targetFlag, 0);
		}

		void bind() const{
			glBindTexture(targetFlag, nameID);
		}

		virtual ~Texture() = default;

		virtual void active(unsigned offset) const = 0;
		virtual void activeAll(unsigned offset) const = 0;
	};
}
