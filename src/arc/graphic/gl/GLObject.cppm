export module GL.Object;

export import GL;
import std;
import GL.Constants;

export namespace GL{
	class GLObject{
	protected:
		GLuint nameID = 0;
		GLenum targetFlag = 0;

		constexpr void reset() noexcept{
			nameID = 0;
			targetFlag = 0;
		}

	public:
		[[nodiscard]] constexpr GLObject() = default;

		constexpr explicit GLObject(const GLenum targetFlag)
			: targetFlag(targetFlag){
		}

		GLObject(const GLObject& other) = delete;

		constexpr GLObject(GLObject&& other) noexcept
			: nameID(other.nameID),
			  targetFlag(other.targetFlag){
			other.reset();
		}

		GLObject& operator=(const GLObject& other) = delete;

		constexpr GLObject& operator=(GLObject&& other) noexcept{
			if(this == &other) return *this;
			nameID = other.nameID;
			targetFlag = other.targetFlag;
			other.reset();
			return *this;
		}

		[[nodiscard]] friend constexpr bool operator==(const GLObject& lhs, const GLObject& rhs) noexcept{
			return lhs.nameID == rhs.nameID
				&& lhs.targetFlag == rhs.targetFlag;
		}

		[[nodiscard]] friend constexpr bool operator!=(const GLObject& lhs, const GLObject& rhs) noexcept{
			return !(lhs == rhs);
		}

		[[nodiscard]] constexpr GLuint getID() const noexcept{
			return nameID;
		}

		[[nodiscard]] constexpr GLenum getTargetFlag() const noexcept{
			return targetFlag;
		}

		[[nodiscard]] constexpr std::size_t hashCode() const noexcept{
			return static_cast<std::size_t>(targetFlag) << 32 | targetFlag;
		}

		friend constexpr void swap(GLObject& lhs, GLObject& rhs) noexcept{
			using std::swap;
			swap(lhs.nameID, rhs.nameID);
			swap(lhs.targetFlag, rhs.targetFlag);
		}

		[[nodiscard]] constexpr bool valid() const noexcept{
			return nameID != 0;
		}
	};
}
export
	template<>
	struct std::hash<GL::GLObject> {
		[[nodiscard]] constexpr std::size_t operator()(const GL::GLObject& obj) const noexcept{
			return obj.hashCode();
		}
	};


