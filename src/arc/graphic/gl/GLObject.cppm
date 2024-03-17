export module GL.Object;

export import GL;
import std;
import GL.Constants;

export namespace GL{
	struct  GLObject{
	protected:
		GLuint nameID = 0;
		GLenum targetFlag = 0;

		void reset(){
			nameID = 0;
			targetFlag = 0;
		}

	public:
		[[nodiscard]] GLObject() = default;

		explicit GLObject(const GLenum targetFlag)
			: targetFlag(targetFlag){
		}

		GLObject(const GLObject& other) = delete;

		GLObject(GLObject&& other) noexcept
			: nameID(other.nameID),
			  targetFlag(other.targetFlag){
			other.reset();
		}

		GLObject& operator=(const GLObject& other) = delete;

		GLObject& operator=(GLObject&& other) noexcept{
			if(this == &other) return *this;
			nameID = other.nameID;
			targetFlag = other.targetFlag;
			other.reset();
			return *this;
		}

		friend bool operator==(const GLObject& lhs, const GLObject& rhs){
			return lhs.nameID == rhs.nameID
				&& lhs.targetFlag == rhs.targetFlag;
		}

		friend bool operator!=(const GLObject& lhs, const GLObject& rhs){
			return !(lhs == rhs);
		}

		[[nodiscard]] GLuint getID() const {
			return nameID;
		}

		[[nodiscard]] GLenum getTargetFlag() const {
			return targetFlag;
		}

		[[nodiscard]] size_t hashCode() const{
			return static_cast<size_t>(targetFlag) << 32 | targetFlag;
		}
	};
}
export
	template<>
	struct std::hash<GL::GLObject> {
		[[nodiscard]] size_t operator()(const GL::GLObject& obj) const noexcept{
			return obj.hashCode();
		}
	};


