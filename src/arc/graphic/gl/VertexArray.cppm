//
// Created by Matrix on 2023/11/18.
//

module ;



export module GL.VertexArray;

import <glad/glad.h>;
import <vector>;
import GL.Buffer.VertexBuffer;

import GL.Constants;
// All VAOs should be constructed at one load function.

export namespace GL{
	struct VertElem{
		GLenum type{0};
		GLboolean normalized{0};
		GLint size{0};
	};

	class AttributeLayout
	{
	protected:
		GLsizei stride = 0;
		std::vector<VertElem> elems{};

	public:
		void add(const GLenum type, const GLint size, const GLboolean normalized = true){
			elems.push_back({ type, normalized, size });
			stride += sizeofType(type) * size;
		}

		void addFloat(const GLint size){
			add(GL_FLOAT, size);
		}

		void generateAttributePointer() const{
			unsigned long long offset = 0;

			for(unsigned int i = 0; i < elems.size(); i++){
				const auto& [type, normalized, size] = elems.at(i);
				glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, size, type, normalized, stride, reinterpret_cast<const void*>(offset));  // NOLINT(performance-no-int-to-ptr)
				offset += static_cast<unsigned long long>(size) * sizeofType(type);
			}
		}
	};


	class VertexArray
	{
	protected:
		GLuint arrayID = 0;
		AttributeLayout layout;

	public:
		VertexArray(const VertexArray& other) = delete;

		VertexArray(VertexArray&& other) = delete;

		VertexArray& operator=(const VertexArray& other) = delete;

		VertexArray& operator=(VertexArray&& other) = delete;


		VertexArray() {
			glGenVertexArrays(1, &arrayID);
		}

		~VertexArray() {
			glDeleteVertexArrays(1, &arrayID);
		}

		[[nodiscard]] GLuint getID() const{
			return arrayID;
		}

		AttributeLayout& getLayout(){
			return layout;
		}

		void bind() const {
			glBindVertexArray(arrayID);
		}

		void unbind() const {
			glBindVertexArray(0);
		}

		void active(/*const VertexBuffer& vertices*/) const{
			bind();
			// vertices.bind();

			layout.generateAttributePointer();

			// vertices.unbind();
			unbind();
		}
	};
}


