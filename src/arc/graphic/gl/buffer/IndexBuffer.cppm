module;

export module GL.Buffer.IndexBuffer;

import <glad/glad.h>;
import GL.Buffer;
import <array>;

export namespace GL{
	class IndexBuffer final : public GLBuffer
	{
	public:
		/**The Count Of Vert Indexes Needed To Draw A Quad*/
		static constexpr size_t ELEMENTS_QUAD_LENGTH = 6;
		/**Inbuilt Indexes*/
		static constexpr std::array<GLuint, ELEMENTS_QUAD_LENGTH> ELEMENTS_STD = {0, 2, 1, 2, 3, 0};

		/**The Count Of Vert Indexes Needed To Draw A Quad*/
		static constexpr size_t ELEMENTS_QUAD_STRIP_LENGTH = 4;
		/**Inbuilt Indexes*/
		static constexpr std::array<GLuint, ELEMENTS_QUAD_STRIP_LENGTH> ELEMENTS_STRIP_STD = {0, 1, 2, 3};

	protected:
		GLsizei bufferSize = 0;

	public:
		IndexBuffer(){
			glGenBuffers(1, &bufferID);
			targetFlag = GL_ELEMENT_ARRAY_BUFFER;
		}

		~IndexBuffer(){
			glDeleteBuffers(1, &bufferID);
		}

		IndexBuffer(const IndexBuffer& other) = delete;

		IndexBuffer(IndexBuffer&& other) = delete;

		IndexBuffer& operator=(const IndexBuffer& other) = delete;

		IndexBuffer& operator=(IndexBuffer&& other) = delete;

		template <GLuint size>
		void setData(GLuint(&arr)[size], const GLenum mode = GL_DYNAMIC_DRAW) {
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * size, &arr, mode);
			bufferSize = size;
		}

		void bind() const{
			glBindBuffer(targetFlag, bufferID);
		}

		void setDataRaw(const GLuint* data, const GLsizei count, const GLenum mode = GL_DYNAMIC_DRAW){
			glBufferData(targetFlag, static_cast<long long>(sizeof(GLuint)) * count, data, mode);
			bufferSize = count;
		}

		void unbind() const{
			glBindBuffer(targetFlag, 0);
		}

		[[nodiscard]] GLsizei getSize() const{
			return bufferSize;
		}
	};
}
