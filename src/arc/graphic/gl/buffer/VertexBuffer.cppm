module;

export module GL.Buffer.VertexBuffer;

import <glad/glad.h>;

export import GL.Object;
import <memory>;

export namespace GL{
	class  VertexBuffer final : public GLObject {
	protected:
		GLsizei bufferSize = 0;
		bool enabledMapping{false};

	public:
		static constexpr GLbitfield
			BasicMappingFlags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
			BasicStorageFlags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;

		VertexBuffer(){
			glCreateBuffers(1, &nameID);
			targetFlag = GL_ARRAY_BUFFER;
		}

		~VertexBuffer() {
			if(nameID){
				disableDataMapping();
				glDeleteBuffers(1, &nameID);
			}

		}

		template <typename T>
		void setData(const T* data, const GLsizei count, const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(nameID, count * sizeof(T), data, mode);
			bufferSize = count;
		}

		template <typename T, GLsizei size>
		void setData(const T(&arr)[size], const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(nameID, size * sizeof(T), &arr, mode);
			bufferSize = size;
		}

		template <typename T>
		void setDataRaw(const T* data, const GLsizei size, const GLenum mode = GL_DYNAMIC_DRAW) {
			glNamedBufferData(nameID, size, data, mode);
			bufferSize = size / sizeof(T);
		}

		void setupStorage(const GLbitfield flag = BasicStorageFlags) const{
			glNamedBufferStorage(nameID, bufferSize * sizeof(float), nullptr, flag);
		}

		void setupStorage(const GLsizei count, const float* data = nullptr, const GLbitfield flag = BasicStorageFlags){
			glNamedBufferStorage(nameID, count * sizeof(float), data, flag);
			bufferSize = count;
		}

		template <typename T = float>
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
}
