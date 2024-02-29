module;

export module Batch_World;

export import Core.Batch;
export import Core.Batch.Batch_Sprite;

import GL;
import GL.Constants;
import GL.Texture.Texture2D;
import <memory>;
import <glad/glad.h>;

export namespace Core {
	template <GLsizei maxVertSize = 8192 * 2, GLsizei textureSize = 2, GLsizei vertGroupSize = GL::VERT_GROUP_SIZE_WORLD>
	class WorldBatch final : public SpriteBatch<maxVertSize, vertGroupSize>{
	public:
		// void flush() override {
		//
		// }

		void post(const ::GL::Texture2D* texture, float* vertices, const int length, const int offset, const int count) override {

		}
	};
}
