//
// Created by Matrix on 2023/11/20.
//
module;

export module Core.Batch.Batch_Sprite;

import <glad/glad.h>;
import <memory>;
import <functional>;
import <span>;

export import Core.Batch;
import GL.Texture.Texture2D;
import GL.GL_Exception;
import GL.Shader;
import GL.Mesh;
import GL.Buffer.IndexBuffer;
import GL.VertexArray;

using namespace GL;

export namespace Core{
	class SpriteBatch final : virtual public Batch
	{
	protected:
		GLsizei maxSize{0};
		std::unique_ptr<float[]> cachedVertices = nullptr;
		std::unique_ptr<GLuint[]> indexRef = nullptr;

	public:
		 SpriteBatch(const unsigned int verticesCount, const std::function<Shader*(const SpriteBatch&)>&shader, std::span<VertElem> layoutElems) : maxSize(verticesCount * VERT_GROUP_SIZE){
			constexpr int ELEM_LEN = IndexBuffer::ELEMENTS_QUAD_LENGTH;

			if (verticesCount > std::numeric_limits<int>::max() / ELEM_LEN - 1)throw GL_Exception{ "Exceed Vertices Count!" };

			cachedVertices = std::unique_ptr<float[]>(new float[maxSize]{0});
			indexRef = std::unique_ptr<GLuint[]>(new GLuint[static_cast<unsigned long long>(verticesCount) * ELEM_LEN]{0});

			for(int i = 0; i < verticesCount; i++){
				for(int j = 0; j < ELEM_LEN; ++j){
					indexRef[j + i * ELEM_LEN] = IndexBuffer::ELEMENTS_STD[j] + i * VERT_GROUP_COUNT;
				}
			}

			mesh = std::make_unique<Mesh>(Mesh{
				[
					&layoutElems,
					indexRef_ = indexRef.get(),
					cachedVertices_ = cachedVertices.get(),
					maxSize_ = verticesCount * VERT_GROUP_SIZE,
					verticesCount
				](const Mesh& mesh){
					mesh.getIndexBuffer().bind();
					mesh.getIndexBuffer().setDataRaw(indexRef_, verticesCount * ELEM_LEN);
					mesh.getVertexBuffer().bind();
					mesh.getVertexBuffer().setDataRaw(cachedVertices_, sizeof(float) * maxSize_);

					//TODO: Uses flexible mode by using attrib names as position reference? or just keep it hard and quick?
					AttributeLayout& layout = mesh.getVertexArray().getLayout();

					if(!layoutElems.empty()){
						for (const auto& [type, normalized, size] : layoutElems) {
							layout.add(type, normalized, size);
						}
					}else{
						//Default Layout

						layout.addFloat(2); //Position 2D
						layout.addFloat(2); //UV offset
						layout.addFloat(4); //mix rgba
						layout.addFloat(4); //src rgba
					}

					mesh.getVertexArray().active();
				}
			});

			generalShader = shader(*this);
		}

		explicit SpriteBatch(const std::function<Shader*(const SpriteBatch&)>& shader) : SpriteBatch(8192 * 2, shader, {}){

		}

		~SpriteBatch() override = default;

		SpriteBatch(const SpriteBatch& other) = delete;

		SpriteBatch(SpriteBatch&& other) = delete;

		SpriteBatch& operator=(const SpriteBatch& other) = delete;

		SpriteBatch& operator=(SpriteBatch&& other) = delete;

		void flush() override{
			if(index == 0)return;

			// glDepthMask(false);

			mesh->bind();
			mesh->getVertexBuffer().setDataRaw(cachedVertices.get(), sizeof(float) * index);

			bindShader();
			applyShader();

			mesh->render(std::min(index / VERT_LENGTH_STD * IndexBuffer::ELEMENTS_QUAD_LENGTH, mesh->getIndexBuffer().getSize()));

			index = 0;
		}

		void post(const Texture2D* texture, float* vertices, const int length, const int offset, const int count) override{
			if(lastTexture != texture){
				flush();
				lastTexture = texture;
			}

			if(index + count > maxSize){
				flush();
			}

			std::memcpy(cachedVertices.get() + index, vertices + offset, count * sizeof(float));

			index += count;
		}
	};
}



