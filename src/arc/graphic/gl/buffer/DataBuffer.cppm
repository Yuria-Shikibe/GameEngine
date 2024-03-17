module;

export module GL.Buffer.DataBuffer;

import <glad/glad.h>;
export import GL.Object;
import std;

export namespace GL{
	/**The Count Of Vert Indexes Needed To Draw A Quad*/
	inline constexpr size_t ELEMENTS_QUAD_LENGTH = 6;
	/**Inbuilt Indexes*/
	inline constexpr std::array<GLuint, ELEMENTS_QUAD_LENGTH> ELEMENTS_STD = {0, 2, 1, 2, 3, 0};

	/**The Count Of Vert Indexes Needed To Draw A Quad*/
	inline constexpr size_t ELEMENTS_QUAD_STRIP_LENGTH = 4;
	/**Inbuilt Indexes*/
	inline constexpr std::array<GLuint, ELEMENTS_QUAD_STRIP_LENGTH> ELEMENTS_STRIP_STD = {0, 1, 2, 3};


	template <GLenum Flag, typename T>
	class DataBuffer : public GLObject{
	protected:
		GLsizei bufferSize = 0;
		bool enabledMapping{false};

	public:
		static constexpr auto InitTargetFlag = Flag;
		static constexpr GLbitfield
			BasicMappingFlags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
			BasicStorageFlags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;

		using DataType = T;

		DataBuffer() : GLObject(Flag){
			glCreateBuffers(1, &nameID);
		}

		~DataBuffer() {
			if(nameID){
				disableDataMapping();
				glDeleteBuffers(1, &nameID);
			}

		}

		template <GLsizei size>
		void setData(const T(&arr)[size], const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(nameID, size * sizeof(T), &arr, mode);
			bufferSize = size;
		}

		void setDataRaw(const T* data, const GLsizei count, const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(nameID, count * sizeof T, data, mode);
			bufferSize = count;
		}

		void setupStorage(const GLsizei count, const float* data = nullptr, const GLbitfield flag = BasicStorageFlags){
			glNamedBufferStorage(nameID, count * sizeof(T), data, flag);
			bufferSize = count;
		}

		[[nodiscard]] T* enableDataMapping(const GLenum access = BasicMappingFlags) const{
			return static_cast<T*>(glMapNamedBuffer(nameID, access));
		}

		void unmap() const{
			glUnmapNamedBuffer(nameID);
		}

		void disableDataMapping(){
			if(enabledMapping){
				glUnmapNamedBuffer(nameID);
				enabledMapping = false;
			}
		}

		[[nodiscard]] GLsizei getSize() const {
			return bufferSize;
		}

		void bind() const {
			GL::bindBuffer(targetFlag, nameID);
		}

		void unbind() const {
			GL::unbindBuffer(targetFlag);
		}
	};

	using VertexBuffer = DataBuffer<GL_ARRAY_BUFFER, float>;
	using IndexBuffer = DataBuffer<GL_ELEMENT_ARRAY_BUFFER, GLuint>;
}
