export module GL.Mesh;

import <glad/glad.h>;
import std;

export import GL.Buffer.DataBuffer;
export import GL.VertexArray;

import Concepts;

export namespace GL{
	struct Mesh //TODO worth virtual ??
	{
		virtual ~Mesh() = default;

	protected:
		VertexBuffer vertexBuffer{};
		IndexBuffer indexBuffer{};
		VertexArray vertexArray{};

	public:
		Mesh() = default;

		explicit Mesh(Concepts::Invokable<void(Mesh&)> auto&& init){
			init(*this);
		}

		[[nodiscard]] VertexBuffer& getVertexBuffer(){
			return vertexBuffer;
		}

		[[nodiscard]] IndexBuffer& getIndexBuffer(){
			return indexBuffer;
		}

		[[nodiscard]] VertexArray& getVertexArray(){
			return vertexArray;
		}

		void applyLayout() const{
			vertexArray.applyLayout(vertexBuffer, indexBuffer);
		}

		void bind() const{
			vertexArray.bind();
			// vertexBuffer.bind();
			// indexBuffer.bind();
		}

		void unbind() const{
			// vertexBuffer.unbind();
			vertexArray.unbind();
			// indexBuffer.unbind();
		}

		template <size_t size>
		Mesh& setVertices(float(&arr)[size]) {
			vertexBuffer.bind();
			vertexBuffer.setData(arr);

			return *this;
		}

		Mesh& setVertices(const float* arr, const int offset, const int count) {
			vertexBuffer.bind();
			vertexBuffer.setDataRaw(arr + offset, count);

			return *this;
		}

		//TODO worth it a virtual?
		virtual void render(const GLenum primitiveType, const int offset, const int count) const{
			glDrawElements(primitiveType, count, GL_UNSIGNED_INT, offset == 0 ? nullptr : reinterpret_cast<const void*>(offset * sizeof(GLuint)));
		}

		virtual void render(const int count) const{
			render(GL_TRIANGLES, 0, count);
		}

		virtual void render() const {
			bind();
			render(indexBuffer.getSize());
		}
	};
}


