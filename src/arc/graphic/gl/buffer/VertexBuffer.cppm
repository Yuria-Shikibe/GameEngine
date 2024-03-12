module;

export module GL.Buffer.VertexBuffer;

import <glad/glad.h>;

export import GL.Buffer;

export namespace GL{
	class  VertexBuffer final : public GLBuffer
	{

	protected:
		GLsizei bufferSize = 0;

	public:
		VertexBuffer(){
			glCreateBuffers(1, &bufferID);
			targetFlag = GL_ARRAY_BUFFER;
		}

		~VertexBuffer() {
			if(bufferID)glDeleteBuffers(1, &bufferID);
		}

		template <typename T>
		void setData(const T* data, const GLsizei count, const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(bufferID, count * sizeof(T), data, mode);
			bufferSize = count;
		}

		template <typename T, GLsizei size>
		void setData(const T(&arr)[size], const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(bufferID, size * sizeof(T), &arr, mode);
			bufferSize = size;
		}

		template <typename T>
		void setDataRaw(const T* data, const GLsizei size, const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(bufferID, size, data, mode);
			bufferSize = size / sizeof(T);
		}

		[[nodiscard]] GLsizei getSize() const {
			return bufferSize;
		}

		void bind() const {
			GL::bindBuffer(targetFlag, bufferID);
		}

		void unbind() const {
			GL::unbindBuffer(targetFlag);
		}
	};
}
