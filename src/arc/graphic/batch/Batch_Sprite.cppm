module;

#include <glad/glad.h>

export module Core.Batch.Batch_Sprite;

import std;
import std.compat;
export import Core.Batch;
import GL.Shader;
import GL.Constants;
import GL.Mesh;
import GL.Buffer.DataBuffer;
import GL.VertexArray;
import GL;
import Math;

import ext.Concepts;

export namespace Core{

	//TODO batch should have their unique shader and manage their lifetime, since the drawer in the shaders may ref the
	//TODO projections of the batch
	template <GLsizei vertGroupSize = GL::VERT_GROUP_SIZE_LAYOUT, GLsizei maxVertSize = 8192 * 2>
		requires requires{requires maxVertSize <= std::numeric_limits<GLsizei>::max() / GL::ELEMENTS_QUAD_LENGTH;}
	class SpriteBatch : public Batch
	{
	public:
		static constexpr auto quadGroupSize = vertGroupSize * GL::QUAD_GROUP_COUNT;
		static constexpr auto maxDataSize = maxVertSize * vertGroupSize;
		static constexpr auto maxIndexSize = maxVertSize * GL::ELEMENTS_QUAD_LENGTH;

	protected:
		GLsync sync;

		// std::array<float, maxDataSize> cachedVertices{};

		static constexpr std::array<GLuint, maxIndexSize> indexRef = {[]{
			std::array<GLuint, maxIndexSize> arr{};

			constexpr int ELEM_LEN = GL::ELEMENTS_QUAD_LENGTH;
			const auto data = GL::ELEMENTS_STD.data();

			for(int i = 0; i < maxVertSize; i++){
				for(int j = 0; j < ELEM_LEN; ++j){
					arr[j + i * ELEM_LEN] = data[j] + i * GL::QUAD_GROUP_COUNT;
				}
			}

			return arr;
		}()};

	public:
		void reset() override{
			Batch::reset();
			index = 0;
		}

		SpriteBatch(Concepts::Invokable<GL::ShaderProgram*(const SpriteBatch&)> auto&& shader, Concepts::Invokable<void(GL::AttributeLayout&)> auto&& layouter){
			mesh1.indexBuffer.setDataRaw(indexRef.data(), indexRef.size(), GL_STATIC_DRAW);
			mesh1.vertexBuffer.allocateStorage(maxDataSize, nullptr,
				GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

			buffer1 = mesh1.vertexBuffer.enableRangedDataMapping(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT  | GL_MAP_FLUSH_EXPLICIT_BIT);
			layouter(mesh1.vertexArray.getLayout());
			mesh1.applyLayout();

			mesh2.indexBuffer.setDataRaw(indexRef.data(), indexRef.size(), GL_STATIC_DRAW);
			mesh2.vertexBuffer.allocateStorage(maxDataSize, nullptr,
				GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

			buffer2 = mesh2.vertexBuffer.enableRangedDataMapping(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT  | GL_MAP_FLUSH_EXPLICIT_BIT);
			layouter(mesh2.vertexArray.getLayout());
			mesh2.applyLayout();

			generalShader = shader(*this);
		}

		explicit SpriteBatch(auto&& shader) : SpriteBatch(shader, [](GL::AttributeLayout& layout){
			layout.addFloat(2);
			layout.addFloat(2);
			layout.addFloat(4);
			layout.addFloat(4);
		}){}

		~SpriteBatch() override{
			glFlushMappedNamedBufferRange(mesh1.vertexBuffer.getID(), 0, mesh1.vertexBuffer.getByteSize());

			mesh1.vertexBuffer.unmap();
		}

		SpriteBatch(const SpriteBatch& other) = delete;

		SpriteBatch(SpriteBatch&& other) = delete;

		SpriteBatch& operator=(const SpriteBatch& other) = delete;

		SpriteBatch& operator=(SpriteBatch&& other) = delete;

		void flush() override{
			if(index == 0)return;

			glFlushMappedNamedBufferRange(getCurrentMesh().vertexBuffer.getID(), 0, index * sizeof(float));
			bindShader();
			applyShader();

			// Delete the sync object (cleanup)
			glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, std::numeric_limits<GLuint64>::max());
			glDeleteSync(sync);

			getCurrentMesh().render(index / quadGroupSize * GL::ELEMENTS_QUAD_LENGTH);

			sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

			swapBuffer();
			index = 0;
		}

		void post(const GL::Texture* texture, float* vertices, const int offset, const int count) override{
			if(lastTexture != texture){
				flush();
				lastTexture = texture;
			}

			if(index + count > maxDataSize){
				flush();
			}

			std::memcpy(getCurrentMappedPtr() + index, vertices + offset, count * sizeof(float));

			index += count;
		}

		void checkFlush(const GL::Texture* texture, const unsigned count) override{
			if(lastTexture != texture){
				flush();
				lastTexture = texture;
			}

			if(index + count > maxDataSize){
				flush();
			}
		}
	};
}



