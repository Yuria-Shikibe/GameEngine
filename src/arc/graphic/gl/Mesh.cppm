module;



export module GL.Mesh;

import <glad/glad.h>;

import <functional>;
import <memory>;
import GL.Buffer.IndexBuffer;
import GL.Shader;
import GL.Buffer.VertexBuffer;
import GL.VertexArray;

export namespace GL{
	struct Mesh
	{

		std::unique_ptr<VertexBuffer> vertexBuffer = nullptr;
		std::unique_ptr<IndexBuffer> indexBuffer = nullptr;
		std::unique_ptr<VertexArray> vertexArray = nullptr;

		//TODO Should Meshes obtain their own shaders?
		//std::unique_ptr<Shader> defaultShader = nullptr;

		Mesh(){
			vertexBuffer = std::make_unique<VertexBuffer>();
			indexBuffer = std::make_unique<IndexBuffer>();
			vertexArray = std::make_unique<VertexArray>();
		}

		explicit Mesh(const auto& init) : Mesh() {
			init(*this);
		}

		Mesh(const Mesh& other) = delete;

		Mesh(Mesh&& other) noexcept{
			vertexBuffer = std::move(other.vertexBuffer);
			indexBuffer = std::move(other.indexBuffer);
			vertexArray = std::move(other.vertexArray);
		}

		Mesh& operator=(const Mesh& other) = delete;

		Mesh& operator=(Mesh&& other) noexcept {
			if(&other == this)return *this;

			vertexBuffer = std::move(other.vertexBuffer);
			indexBuffer = std::move(other.indexBuffer);
			vertexArray = std::move(other.vertexArray);

			return *this;
		}

		[[nodiscard]] VertexBuffer& getVertexBuffer() const{
			return *vertexBuffer;
		}

		[[nodiscard]] IndexBuffer& getIndexBuffer() const{
			return *indexBuffer;
		}

		[[nodiscard]] VertexArray& getVertexArray() const{
			return *vertexArray;
		}

		void bind() const{
			vertexBuffer->bind();
			indexBuffer->bind();
			vertexArray->bind();
		}

		template <size_t size>
		Mesh& setVertices(float(&arr)[size]) {
			vertexBuffer->bind();

			vertexBuffer->setData(arr);

			return *this;
		}

		Mesh& setVertices(const float* arr, const int offset, const int count) {
			vertexBuffer->bind();

			vertexBuffer->setData(arr + offset, count);

			return *this;
		}

		void render(const GLenum primitiveType, const int offset, const int count) const{
			if (indexBuffer != nullptr && indexBuffer->getSize() > 0) {
				glDrawElements(primitiveType, count, GL_UNSIGNED_INT, reinterpret_cast<const void*>(offset * sizeof(GLuint)));
			}
			else {
				glDrawArrays(primitiveType, offset, count);
			}
		}

		void render(const int count) const{
			render(GL_TRIANGLES, 0, count);
		}
	};
}


