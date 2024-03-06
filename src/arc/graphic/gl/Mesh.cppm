module;



export module GL.Mesh;

import <glad/glad.h>;

import <functional>;
import <memory>;
import GL.Buffer.IndexBuffer;
import GL.Buffer.VertexBuffer;
import GL.VertexArray;

import Concepts;

export namespace GL{
	struct Mesh
	{
		virtual ~Mesh() = default;

		std::unique_ptr<VertexBuffer> vertexBuffer{std::make_unique<VertexBuffer>()};
		std::unique_ptr<IndexBuffer> indexBuffer{std::make_unique<IndexBuffer>()};
		std::unique_ptr<VertexArray> vertexArray{std::make_unique<VertexArray>()};

		Mesh() = default;

		explicit Mesh(Concepts::Invokable<void(Mesh&)> auto&& init){
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

		virtual void bind() const{
			vertexArray->bind();
			vertexBuffer->bind();
			indexBuffer->bind();
		}

		virtual void unbind() const{
			vertexBuffer->unbind();
			vertexArray->unbind();
			indexBuffer->unbind();
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

		//TODO worth it a virtual?
		virtual void render(const GLenum primitiveType, const int offset, const int count) const{

			if (indexBuffer != nullptr && indexBuffer->getSize() > 0) {
				glDrawElements(primitiveType, count, GL_UNSIGNED_INT, offset == 0 ? nullptr : reinterpret_cast<const void*>(offset * sizeof(GLuint)));
			}else {
				glDrawArrays(primitiveType, offset, count);
			}
		}

		virtual void render(const int count) const{
			render(GL_TRIANGLES, 0, count);
		}

		virtual void render() const {
			bind();
			if(indexBuffer && indexBuffer->getSize() > 0) { // NOLINT(*-branch-clone)
				render(indexBuffer->getSize());
			}else{
				render(vertexBuffer->getSize());
			}
		}
	};
}


