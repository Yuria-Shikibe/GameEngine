//
// Created by Matrix on 2024/3/12.
//

export module GL.UniformBuffer;

import <glad/glad.h>;
import <vector>;
export import GL.Buffer.DataBuffer;

export namespace GL{
	struct UniformElem {
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
