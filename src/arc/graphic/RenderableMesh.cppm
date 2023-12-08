//
// Created by Matrix on 2023/11/30.
//

module;

export module GL.Mesh.RenderableMesh;

export import GL.Mesh;
import GL.Shader;
import RuntimeException;
import <glad/glad.h>;

export namespace GL {
	struct RenderableMesh final : Mesh {
	protected:
		const Shader* shader = nullptr;
		GLenum primitiveType = GL_TRIANGLE_FAN;

	public:
		~RenderableMesh() override = default;

		explicit RenderableMesh(const Shader* const shader, const auto& init) : shader(shader){
			init(*this);
		}

		using Mesh::render;

		void bind() const override {
			shader->bind();

			Mesh::bind();
		}

		void unbind() const override {
			shader->bind();

			Mesh::unbind();
		}

		void render() const override {
			if(!shader)throw ext::RuntimeException{"Empty Shader Applied to a RenderableMesh!!"};

			shader->bind();
			shader->apply();
			Mesh::render();
		}

		void render(const int count) const override{
			render(primitiveType, 0, count);
		}
	};
}
