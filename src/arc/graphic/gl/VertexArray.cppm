module;

#include <glad/glad.h>

export module GL.VertexArray;

import std;
import GL.Buffer.DataBuffer;
import GL.Object;
import GL.Constants;
// All VAOs should be constructed at one load function.

export namespace GL{
	struct VertElem{
		GLenum type{0};
		GLboolean normalized{0};
		GLint size{0};
	};

	class AttributeLayout //TODO make this impl by template and finish layout in compile
	{
	protected:
		GLsizei stride = 0;
		std::vector<VertElem> elems{};

	public:
		void add(const GLenum type, const GLint size, const GLboolean normalized = false){
			elems.emplace_back( type, normalized, size );
			stride += GL::sizeofType(type) * size;
		}

		void addFloat(const GLint size){
			add(GL_FLOAT, size);
		}

		[[nodiscard]] GLsizei getStride() const{
			return stride;
		}

		/**
		 * @brief VAO, IBO must have binded!!!
		 */
		void generateAttributePointer(const GLuint vao) const{
			GLuint offset = 0;

			for(GLuint i = 0; i < elems.size(); i++){
				const auto& [type, normalized, size] = elems.at(i);
				glEnableVertexArrayAttrib(vao, i);
				glVertexArrayAttribFormat(vao, i, size, type, normalized, offset);
				glVertexArrayAttribBinding(vao, i, 0);
				offset += size * sizeofType(type);
			}
		}
	};


	class VertexArray
	{
	protected:
		GLuint vaoID = 0;
		AttributeLayout layout{};

	public:
		VertexArray(const VertexArray& other) = delete;

		VertexArray(VertexArray&& other) noexcept
			: vaoID(other.vaoID),
			  layout(std::move(other.layout)){
			other.vaoID = 0;
		}

		VertexArray& operator=(const VertexArray& other) = delete;

		VertexArray& operator=(VertexArray&& other) noexcept{
			if(this == &other) return *this;
			vaoID = other.vaoID;
			layout = std::move(other.layout);
			other.vaoID = 0;
			return *this;
		}

		VertexArray() {
			glCreateVertexArrays(1, &vaoID);
		}

		~VertexArray() {
			if(vaoID)glDeleteVertexArrays(1, &vaoID);
		}

		[[nodiscard]] GLuint getID() const{
			return vaoID;
		}

		AttributeLayout& getLayout(){
			return layout;
		}

		void bindBuffer(const GL::GLObject& VBO, const GL::GLObject& IBO) const {
			glVertexArrayVertexBuffer(vaoID, 0, VBO.getID(), 0, layout.getStride());
			glVertexArrayElementBuffer(vaoID, IBO.getID());
		}

		void bind() const {
			glBindVertexArray(vaoID);
		}

		void unbind() const {
			glBindVertexArray(0);
		}

		void applyLayout(const GL::GLObject& VBO, const GL::GLObject& IBO) const{
			bindBuffer(VBO, IBO);
			layout.generateAttributePointer(vaoID);
		}
	};
}


