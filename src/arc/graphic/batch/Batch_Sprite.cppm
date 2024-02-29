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

	//TODO uses template constval as max size to avoid heap data allocation
	template <GLsizei maxVertSize = 8192 * 2, GLsizei vertGroupSize = GL::VERT_GROUP_SIZE_LAYOUT>
		requires requires{maxVertSize <= std::numeric_limits<GLsizei>::max() / IndexBuffer::ELEMENTS_QUAD_LENGTH;}
	class SpriteBatch : public Batch
	{
	public:
		static constexpr auto maxDataSize = maxVertSize * vertGroupSize;
		static constexpr auto maxIndexSize = static_cast<unsigned long long>(maxVertSize) * IndexBuffer::ELEMENTS_QUAD_LENGTH;

	protected:
		std::array<float, maxDataSize> cachedVertices{};
		std::array<GLuint, maxIndexSize> indexRef{};

	public:
		template <Concepts::Invokable<Shader*(const SpriteBatch&)> Func>
		SpriteBatch(Func&& shader, std::span<VertElem> layoutElems){
			constexpr int ELEM_LEN = IndexBuffer::ELEMENTS_QUAD_LENGTH;

			for(int i = 0; i < maxVertSize; i++){
				for(int j = 0; j < ELEM_LEN; ++j){
					indexRef[j + i * ELEM_LEN] = IndexBuffer::ELEMENTS_STD[j] + i * VERT_GROUP_COUNT;
				}
			}

			mesh = std::make_unique<Mesh>(Mesh{
				[
					&layoutElems,
					this,
					maxSize_ = maxVertSize * vertGroupSize
				](const Mesh& mesh){
					mesh.getIndexBuffer().bind();
					mesh.getIndexBuffer().setDataRaw(indexRef.data(), maxVertSize * ELEM_LEN);
					mesh.getVertexBuffer().bind();
					mesh.getVertexBuffer().setDataRaw(cachedVertices.data(), sizeof(float) * maxSize_);

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

			mesh->render(std::min(index / VERT_LENGTH_STD * IndexBuffer::ELEMENTS_QUAD_LENGTH, static_cast<size_t>(mesh->getIndexBuffer().getSize())));

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



