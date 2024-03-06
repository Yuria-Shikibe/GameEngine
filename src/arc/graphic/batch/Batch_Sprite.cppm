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
import GL.Constants;
import GL.Mesh;
import GL.Buffer.IndexBuffer;
import GL.VertexArray;

import Concepts;

using namespace GL;

export namespace Core{

	template <GLsizei maxVertSize = 8192 * 2, GLsizei vertGroupSize = GL::VERT_GROUP_SIZE_LAYOUT>
		requires requires{maxVertSize <= std::numeric_limits<GLsizei>::max() / GL::ELEMENTS_QUAD_LENGTH;}
	class SpriteBatch : public Batch
	{
	public:
		static constexpr auto maxDataSize = maxVertSize * vertGroupSize;
		static constexpr auto maxIndexSize = static_cast<unsigned long long>(maxVertSize) * GL::ELEMENTS_QUAD_LENGTH;

	protected:
		std::array<float, maxDataSize> cachedVertices{};
		static constexpr std::array<GLuint, maxIndexSize> indexRef = {[]{
			std::array<GLuint, maxIndexSize> arr{};

			constexpr int ELEM_LEN = GL::ELEMENTS_QUAD_LENGTH;
			const auto data = GL::ELEMENTS_STD.data();

			for(int i = 0; i < maxVertSize; i++){
				for(int j = 0; j < ELEM_LEN; ++j){
					arr[j + i * ELEM_LEN] = data[j] + i * VERT_GROUP_COUNT;
				}
			}

			return arr;
		}()};

	public:
		SpriteBatch(Concepts::Invokable<Shader*(const SpriteBatch&)> auto&& shader, std::span<VertElem> layoutElems){
			mesh = std::make_unique<Mesh>(Mesh{
				[
					&layoutElems,
					this
				](const Mesh& mesh){
					mesh.getIndexBuffer().bind();
					mesh.getIndexBuffer().setDataRaw(this->indexRef.data(), this->indexRef.size());
					mesh.getVertexBuffer().bind();
					mesh.getVertexBuffer().setDataRaw(this->cachedVertices.data(), sizeof(float) * maxVertSize * vertGroupSize);

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

					mesh.getVertexArray().applyLayout();
				}
			});

			generalShader = shader(*this);
		}

		explicit SpriteBatch(auto&& shader) : SpriteBatch(shader, {}){

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
			mesh->getVertexBuffer().setDataRaw(cachedVertices.data(), sizeof(float) * index);

			bindShader();
			applyShader();

			mesh->render(std::min(index / VERT_LENGTH_STD * GL::ELEMENTS_QUAD_LENGTH, static_cast<size_t>(mesh->getIndexBuffer().getSize())));

			index = 0;
		}

		void post(const Texture2D* texture, float* vertices, const int length, const int offset, const int count) override{
			if(lastTexture != texture){
				flush();
				lastTexture = texture;
			}

			if(index + count > maxDataSize){
				flush();
			}

			std::memcpy(cachedVertices.data() + index, vertices + offset, count * sizeof(float));

			index += count;
		}
	};
}



