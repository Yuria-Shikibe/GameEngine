export module GL.Buffer;

export import GL.Constants;

export namespace GL{
	class GLBuffer
	{

	protected:
		GLuint bufferID = 0;
		GLenum targetFlag = 0;

	public:
		[[nodiscard]] GLBuffer() = default;

		GLBuffer(const GLBuffer& other) = delete;

		GLBuffer(GLBuffer&& other) = delete;

		GLBuffer& operator=(const GLBuffer& other) = delete;

		GLBuffer& operator=(GLBuffer&& other) = delete;

		[[maybe_unused]] [[nodiscard]] GLuint getID() const{
			return bufferID;
		}

		[[nodiscard]] GLenum getTargetFlag() const {
			return targetFlag;
		}
	};
}


