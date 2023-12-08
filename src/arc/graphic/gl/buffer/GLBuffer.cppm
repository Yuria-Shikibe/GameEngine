export module GL.Buffer;

export import GL.Constants;

export namespace GL{
	class GLBuffer
	{

	protected:
		GLuint bufferID = 0;
		GLenum targetFlag = 0;

	public:
		GLBuffer(const GLBuffer& other) = delete;

		GLBuffer(GLBuffer&& other) = delete;

		GLBuffer& operator=(const GLBuffer& other) = delete;

		GLBuffer& operator=(GLBuffer&& other) = delete;

		virtual ~GLBuffer() = default;
		GLBuffer() = default;

		[[maybe_unused]] virtual void bind() const = 0;

		[[maybe_unused]] virtual void unbind() const = 0;

		[[maybe_unused]] [[nodiscard]] virtual GLuint getID() const{
			return bufferID;
		}

		[[nodiscard]] GLenum getTargetFlag() const {
			return targetFlag;
		}
	};
}


