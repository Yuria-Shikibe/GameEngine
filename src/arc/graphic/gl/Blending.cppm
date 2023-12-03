module ;

export module GL.Blending;

import <glad/glad.h>;
import GL;

export namespace GL{
	class Blending
	{
	public:
		GLenum src, dst, srcAlpha, dstAlpha;

		Blending(const GLenum src, const GLenum dst, const GLenum srcAlpha, const GLenum dstAlpha): src(src),
		                                                                                            dst(dst),
		                                                                                            srcAlpha(srcAlpha),
		                                                                                            dstAlpha(dstAlpha){
		}

		Blending(const Blending& other) = default;

		Blending(Blending&& other) noexcept: src(other.src),
		                                     dst(other.dst),
		                                     srcAlpha(other.srcAlpha),
		                                     dstAlpha(other.dstAlpha){
		}

		Blending& operator=(const Blending& other){
			if(this == &other) return *this;
			src = other.src;
			dst = other.dst;
			srcAlpha = other.srcAlpha;
			dstAlpha = other.dstAlpha;
			return *this;
		}

		Blending& operator=(Blending&& other) noexcept{
			if(this == &other) return *this;
			src = other.src;
			dst = other.dst;
			srcAlpha = other.srcAlpha;
			dstAlpha = other.dstAlpha;
			return *this;
		}

		friend bool operator==(const Blending& lhs, const Blending& rhs){
			return &lhs == &rhs;
		}

		friend bool operator!=(const Blending& lhs, const Blending& rhs){
			return !(lhs == rhs);
		}

		Blending(const GLenum src, const GLenum dst) : Blending(src, dst, src, dst) {}

		~Blending() = default;

		friend bool operator==(const Blending& lhs, const Blending& rhs);

		friend bool operator!=(const Blending& lhs, const Blending& rhs);

		void apply() const{
			GL::enable(GL_BLEND);
			glBlendFuncSeparate(src, dst, srcAlpha, dstAlpha);
		}
	};

	namespace Blendings{
		inline const Blending NORMAL{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
		inline const Blending ADDICTIVE{ GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
		inline const Blending ALPHA_SUSTAIN{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };
	}
}


