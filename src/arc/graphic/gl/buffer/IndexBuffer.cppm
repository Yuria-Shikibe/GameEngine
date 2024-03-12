module;

export module GL.Buffer.IndexBuffer;

import <glad/glad.h>;
import GL.Buffer;
import <array>;

export namespace GL{
	/**The Count Of Vert Indexes Needed To Draw A Quad*/
	inline constexpr size_t ELEMENTS_QUAD_LENGTH = 6;
	/**Inbuilt Indexes*/
	inline constexpr std::array<GLuint, ELEMENTS_QUAD_LENGTH> ELEMENTS_STD = {0, 2, 1, 2, 3, 0};

	/**The Count Of Vert Indexes Needed To Draw A Quad*/
	inline constexpr size_t ELEMENTS_QUAD_STRIP_LENGTH = 4;
	/**Inbuilt Indexes*/
	inline constexpr std::array<GLuint, ELEMENTS_QUAD_STRIP_LENGTH> ELEMENTS_STRIP_STD = {0, 1, 2, 3};

	class IndexBuffer final : public GLBuffer
	{
	protected:
		GLsizei bufferSize = 0;

	public:
		IndexBuffer(){
			glCreateBuffers(1, &bufferID);
			targetFlag = GL_ELEMENT_ARRAY_BUFFER;
		}

		~IndexBuffer(){
			if(bufferID)glDeleteBuffers(1, &bufferID);
		}

		template <GLuint size>
		void setData(GLuint(&arr)[size], const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(bufferID, sizeof(GLuint) * size, &arr, mode);
			bufferSize = size;
		}

		void setDataRaw(const GLuint* data, const GLsizei count, const GLenum mode = GL_DYNAMIC_DRAW){
			glNamedBufferData(bufferID, static_cast<long long>(sizeof(GLuint)) * count, data, mode);
			bufferSize = count;
		}

		[[nodiscard]] GLsizei getSize() const{
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
