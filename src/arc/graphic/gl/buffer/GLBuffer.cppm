export module GL.Buffer;

export import GL.Constants;
export import GL;

export namespace GL{
	struct  GLBuffer{
	protected:
		GLuint bufferID = 0;
		GLenum targetFlag = 0;

		void reset(){
			bufferID = 0;
			targetFlag = 0;
		}

	public:
		[[nodiscard]] GLBuffer() = default;

		GLBuffer(const GLBuffer& other) = delete;

		GLBuffer(GLBuffer&& other) noexcept
			: bufferID(other.bufferID),
			  targetFlag(other.targetFlag){
			other.reset();
		}

		GLBuffer& operator=(const GLBuffer& other) = delete;

		GLBuffer& operator=(GLBuffer&& other) noexcept{
			if(this == &other) return *this;
			bufferID = other.bufferID;
			targetFlag = other.targetFlag;
			other.reset();
			return *this;
		}

		[[nodiscard]] GLuint getID() const{
			return bufferID;
		}

		[[nodiscard]] GLenum getTargetFlag() const {
			return targetFlag;
		}
	};
}


