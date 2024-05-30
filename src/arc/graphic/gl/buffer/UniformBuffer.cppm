module;

#include <glad/glad.h>

export module GL.UniformBuffer;

import std;
export import GL.Buffer.DataBuffer;

export namespace GL{
	struct [[deprecated("Incomplete")]] UniformElem {
		GLenum type{};
		GLuint align{};

		explicit UniformElem(const GLenum type)
			: type(type){
		}
	};

	struct UniformLayout {
		std::vector<UniformElem> elems;
		GLsizei lastSize;

		void add(const GLenum type, const GLsizei count = 1){
			elems.emplace_back(type);

		}


	};

	class UniformBuffer : public DataBuffer<GL_UNIFORM_BUFFER, GLuint> {

	};
}
