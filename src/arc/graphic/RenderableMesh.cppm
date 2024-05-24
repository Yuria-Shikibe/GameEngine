//
// Created by Matrix on 2023/11/30.
//

module;

export module GL.Mesh.RenderableMesh;

export import GL.Mesh;
import GL.Shader;
import ext.RuntimeException;
import GL.Constants;

export namespace GL {
	template <GLenum primitiveType = GL_TRIANGLES>
	struct RenderableMesh final : Mesh {
	protected:
		const ShaderSource* shader = nullptr;
	public:
		~RenderableMesh() override = default;

		explicit RenderableMesh(const ShaderSource* const shader, Concepts::Invokable<void(RenderableMesh&)> auto&& init) : shader(shader){
			init(*this);
		}

		using Mesh::render;

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
