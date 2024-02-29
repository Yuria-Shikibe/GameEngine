module;

export module GL.Buffer.VertexBuffer;

import <glad/glad.h>;

export import GL.Buffer;

export namespace GL{
	class VertexBuffer final : public GLBuffer
	{

	protected:
		GLsizei bufferSize = 0;

	public:
		VertexBuffer() {
			glGenBuffers(1, &bufferID);
			targetFlag = GL_ARRAY_BUFFER;
		}

		~VertexBuffer() {
			glDeleteBuffers(1, &bufferID);
		}

		VertexBuffer(const VertexBuffer& other) = delete;

		VertexBuffer(VertexBuffer&& other) = delete;

		VertexBuffer& operator=(const VertexBuffer& other) = delete;

		VertexBuffer& operator=(VertexBuffer&& other) = delete;

		void bind() const {
			glBindBuffer(targetFlag, bufferID);
		}

		template <typename T>
		void setData(const T* data, const GLsizei count, const GLenum mode = GL_DYNAMIC_DRAW) {
			glBufferData(targetFlag, count * sizeof(T), data, mode);
			bufferSize = count;
		}

		template <typename T, GLsizei size>
		void setData(const T(&arr)[size], const GLenum mode = GL_DYNAMIC_DRAW) {
			glBufferData(targetFlag, size * sizeof(T), &arr, mode);
			bufferSize = size;
		}

		template <typename T>
		void setDataRaw(const T* data, const GLsizei size, const GLenum mode = GL_DYNAMIC_DRAW) {
			glBufferData(targetFlag, size, data, mode);
			bufferSize = size / sizeof(T);
		}

		[[nodiscard]] GLsizei getSize() const {
			return bufferSize;
		}

		void unbind() const {
			glBindBuffer(targetFlag, 0);
		}
	};
}
