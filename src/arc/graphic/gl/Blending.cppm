module;

export module GL.Blending;

import GL.Constants;
import GL;
import std;

export namespace GL{
	enum struct blend : unsigned short{
		disable             = static_cast<unsigned short>(~0u),
		zero                = GL_ZERO,                //(0,0,0,0)
		one                 = GL_ONE,                 //(1, 1, 1, 1)
		src_color           = GL_SRC_COLOR,           //(R / kR, G / kG, B / kB, A / kA )
		one_minus_src_color = GL_ONE_MINUS_SRC_COLOR, //(1, 1, 1, 1) - (R / kR, G / kG, B / kB, A / kA )
		dst_color           = GL_DST_COLOR,           //(Rd / kR, Gd / kG, Bd / kB, Ad / kA )
		one_minus_dst_color = GL_ONE_MINUS_DST_COLOR, //(1, 1, 1, 1) - (Rd / kR、Gd / kG、Bd / k B、Ad / kA )
		src_alpha           = GL_SRC_ALPHA,           //(A / kA, A / kA, A / kA, A / kA )
		one_minus_src_alpha = GL_ONE_MINUS_SRC_ALPHA, //(1, 1, 1, 1) - (A / kA, A / kA, A / kA, A / kA )
		dst_alpha           = GL_DST_ALPHA,           //(Ad / kA, Ad / kA, Ad / kA, A d / kA )
		one_minus_dst_alpha = GL_ONE_MINUS_DST_ALPHA, //(1, 1, 1, 1) - (Ad / kA, Ad / kA, Ad / kA, Ad / kA )
		saturate            = GL_SRC_ALPHA_SATURATE,  //(i, i, i, 1)
	};

	struct Blending{
		blend src{}, dst{}, srcAlpha{}, dstAlpha{};

		[[nodiscard]] constexpr Blending(const blend src, const blend dst, const blend srcAlpha, const blend dstAlpha): src(src),
			dst(dst),
			srcAlpha(srcAlpha),
			dstAlpha(dstAlpha){}

		[[nodiscard]] constexpr Blending(const blend src, const blend dst) : Blending(src, dst, src, dst){}

		[[nodiscard]] constexpr Blending() = default;

		friend constexpr bool operator==(const Blending& lhs, const Blending& rhs){
			return lhs.src == rhs.src
				&& lhs.dst == rhs.dst
				&& lhs.srcAlpha == rhs.srcAlpha
				&& lhs.dstAlpha == rhs.dstAlpha;
		}

		friend constexpr bool operator!=(const Blending& lhs, const Blending& rhs){ return !(lhs == rhs); }

		friend constexpr void swap(Blending& lhs, Blending& rhs) noexcept{
			using std::swap;
			swap(lhs.src, rhs.src);
			swap(lhs.dst, rhs.dst);
			swap(lhs.srcAlpha, rhs.srcAlpha);
			swap(lhs.dstAlpha, rhs.dstAlpha);
		}

		void apply() const{
			if(src == blend::disable || dst == blend::disable || srcAlpha == blend::disable || dstAlpha ==
				blend::disable)[[unlikely]] {
				GL::disable(GL::State::BLEND);
			} else{
				GL::enable(GL::State::BLEND);
				GL::blendFunc(static_cast<GLenum>(src), static_cast<GLenum>(dst), static_cast<GLenum>(srcAlpha),
				              static_cast<GLenum>(dstAlpha));
			}
		}

		void apply(const GLuint id) const{
			GL::enablei(GL::State::BLEND, id);
			GL::blendFunci(id, static_cast<GLenum>(src), static_cast<GLenum>(dst), static_cast<GLenum>(srcAlpha),
			               static_cast<GLenum>(dstAlpha));
		}
	};


	namespace Blendings{
		inline constexpr Blending Disable{blend::disable, blend::disable, blend::disable, blend::disable};
		inline constexpr Blending Normal{
				blend::src_alpha,
				blend::one_minus_src_alpha,
				blend::one,
				blend::one_minus_src_alpha
			};
		inline constexpr Blending Addictive{
				blend::src_alpha,
				blend::one,
				blend::one,
				blend::one_minus_src_alpha
			};
		inline constexpr Blending Inverse{
				blend::one_minus_dst_color,
				blend::one_minus_src_alpha,
				blend::one,
				blend::one_minus_src_alpha
			};
		inline constexpr Blending Multiply{
				blend::dst_color,
				blend::zero,
				blend::src_alpha,
				blend::one_minus_src_alpha
			};
	}
}
