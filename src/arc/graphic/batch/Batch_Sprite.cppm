module;

export module Core.Batch.Batch_Sprite;

import std;
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
		std::array<float, maxDataSize> cachedVertices{};
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
			mesh = std::make_unique<GL::Mesh>([&layouter, this](GL::Mesh& mesh){
				mesh.getIndexBuffer().setDataRaw(this->indexRef.data(), this->indexRef.size(), GL_STATIC_DRAW);
				mesh.getVertexBuffer().setDataRaw(this->cachedVertices.data(), maxDataSize);

				layouter(mesh.getVertexArray().getLayout());

				mesh.applyLayout();
			});

			generalShader = shader(*this);
		}

		explicit SpriteBatch(auto&& shader) : SpriteBatch(shader, [](GL::AttributeLayout& layout){
			layout.addFloat(2);
			layout.addFloat(2);
			layout.addFloat(4);
			layout.addFloat(4);
		}){}

		~SpriteBatch() override = default;

		SpriteBatch(const SpriteBatch& other) = delete;

		SpriteBatch(SpriteBatch&& other) = delete;

		SpriteBatch& operator=(const SpriteBatch& other) = delete;

		SpriteBatch& operator=(SpriteBatch&& other) = delete;

		void flush() override{
			if(index == 0)return;

			mesh->bind();
			mesh->getVertexBuffer().setDataRaw(cachedVertices.data(), index);

			bindShader();
			applyShader();

			mesh->render(Math::min(index / quadGroupSize * GL::ELEMENTS_QUAD_LENGTH, static_cast<std::size_t>(this->indexRef.size())));

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

			std::memcpy(cachedVertices.data() + index, vertices + offset, count * sizeof(float));

			index += count;
		}
	};
}



