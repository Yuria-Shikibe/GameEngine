module;

#include <glad/glad.h>

export module GL.Mesh;

import std;

export import GL.Buffer.DataBuffer;
export import GL.VertexArray;

import ext.Concepts;

export namespace GL{
	struct Mesh
	{
		VertexBuffer vertexBuffer{};
		IndexBuffer indexBuffer{};
		VertexArray vertexArray{};

		Mesh() = default;

		explicit Mesh(Concepts::Invokable<void(Mesh&)> auto&& init){
			init(*this);
		}

		void applyLayout() const{
			vertexArray.applyLayout(vertexBuffer, indexBuffer);
		}

		void bind() const{
			vertexArray.bind();
		}

		void unbind() const{
			vertexArray.unbind();
		}

		template <size_t size>
		Mesh& setVertices(float(&arr)[size]) {
			vertexBuffer.setData(arr);

			return *this;
		}

		Mesh& setVertices(const float* arr, const int offset, const int count) {
			vertexBuffer.setDataRaw(arr + offset, count);

			return *this;
		}

		void render(const GLenum primitiveType, const int offset, const int count) const{
			bind();
			glDrawElements(primitiveType, count, GL_UNSIGNED_INT, offset == 0 ? nullptr : reinterpret_cast<const void*>(offset * sizeof(GLuint)));
		}

		void render(const int count) const{
			render(GL_TRIANGLES, 0, count);
		}

		void render() const {
			render(indexBuffer.getSize());
		}
	};
}


