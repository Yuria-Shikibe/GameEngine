module ;

export module GL.Blending;

import <glad/glad.h>;
import GL;

export namespace GL{
	struct Blending
	{
		GLenum src{}, dst{}, srcAlpha{}, dstAlpha{};

		constexpr Blending(const GLenum src, const GLenum dst, const GLenum srcAlpha, const GLenum dstAlpha): src(src),
		                                                                                            dst(dst),
		                                                                                            srcAlpha(srcAlpha),
		                                                                                            dstAlpha(dstAlpha){
		}

		[[nodiscard]] Blending() = default;

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

		constexpr Blending(const GLenum src, const GLenum dst) : Blending(src, dst, src, dst) {}

		virtual ~Blending() = default;

		friend bool operator==(const Blending& lhs, const Blending& rhs);

		friend bool operator!=(const Blending& lhs, const Blending& rhs);

		virtual void apply() const{
			GL::enable(GL_BLEND);
			GL::blendFunc(src, dst, srcAlpha, dstAlpha);
		}

		virtual void apply(const GLuint id) const{
			GL::enablei(GL_BLEND, id);
			GL::blendFunci(id, src, dst, srcAlpha, dstAlpha);
		}
	};

	struct BlendingDisable final : Blending{
		[[nodiscard]] constexpr BlendingDisable() = default;

		void apply() const override {
			GL::disable(GL_BLEND);
		}

		void apply(const GLuint id) const override {
			GL::disablei(GL_BLEND, id);
		}
	};

	struct BlendingEasy final : Blending{
		constexpr BlendingEasy(const GLenum src, const GLenum dst) : Blending(src, dst, src, dst) {}

		void apply() const override{
			GL::enable(GL_BLEND);
			GL::blendFunc(src, dst);
		}

		void apply(const GLuint id) const override{
			GL::enablei(GL_BLEND, id);
			GL::blendFunci(id, src, dst);
		}
	};

	namespace Blendings{
		inline constexpr BlendingDisable Disable{};
		inline constexpr Blending Normal{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
		inline constexpr Blending Addictive{ GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
		// inline const Blending AlphaSustain{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };
		inline constexpr Blending Inverse{ GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA};
	}
}


