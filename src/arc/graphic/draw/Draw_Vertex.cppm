export module Graphic.Draw:Vertex;

import Graphic.Color;
import Core.Batch;
export import Core.BatchGroup;
export import Core.Batch;
import std;
import GL.Constants;

namespace Graphic::Vertex{
	namespace World{
		void vert(float* vertices,
		          float x1, float y1, float z1, float u1, float v1, Color c1, Color cm1,
		          float x2, float y2, float z2, float u2, float v2, Color c2, Color cm2,
		          float x3, float y3, float z3, float u3, float v3, Color c3, Color cm3,
		          float x4, float y4, float z4, float u4, float v4, Color c4, Color cm4
		);

		void vert_monochromeMix(float* vertices, Color cm,
		                        float x1, float y1, float z1, float u1, float v1, Color c1,
		                        float x2, float y2, float z2, float u2, float v2, Color c2,
		                        float x3, float y3, float z3, float u3, float v3, Color c3,
		                        float x4, float y4, float z4, float u4, float v4, Color c4
		);

		void vert_monochromeAll(float* vertices, Color c, Color cm,
		                        float x1, float y1, float z1, float u1, float v1,
		                        float x2, float y2, float z2, float u2, float v2,
		                        float x3, float y3, float z3, float u3, float v3,
		                        float x4, float y4, float z4, float u4, float v4
		);
	}


	namespace Overlay{
		void vert(float* vertices,
		          float x1, float y1, float u1, float v1, Color c1, Color cm1,
		          float x2, float y2, float u2, float v2, Color c2, Color cm2,
		          float x3, float y3, float u3, float v3, Color c3, Color cm3,
		          float x4, float y4, float u4, float v4, Color c4, Color cm4
		);

		void vert_monochromeMix(float* vertices, Color cm,
		                        float x1, float y1, float u1, float v1, Color c1,
		                        float x2, float y2, float u2, float v2, Color c2,
		                        float x3, float y3, float u3, float v3, Color c3,
		                        float x4, float y4, float u4, float v4, Color c4
		);

		void vert_monochromeAll(float* vertices, Color c, Color cm,
		                        float x1, float y1, float u1, float v1,
		                        float x2, float y2, float u2, float v2,
		                        float x3, float y3, float u3, float v3,
		                        float x4, float y4, float u4, float v4
		);
	}


	export{
		using BatchPtr = std::unique_ptr<Core::Batch>;

		constexpr auto BatchOverlay = &Core::BatchGroup::overlay;
		constexpr auto BatchWorld = &Core::BatchGroup::world;
		constexpr auto DefBatch = BatchOverlay;

		BatchPtr& getBatch(BatchPtr Core::BatchGroup::* batchPtr);

		template <BatchPtr Core::BatchGroup::* ptr>
		struct VertexPasser;
	}

	struct WorldPass{
		static constexpr auto batchPtr = &Core::BatchGroup::world;

		static constexpr int size = GL::VERT_LENGTH_WORLD;

		inline static float vertices[size]{};

		static Core::Batch& getBatch();

		static void vert(const GL::Texture* texture, const auto... args){
			getBatch().checkFlush(texture, size);
			World::vert(getBatch().getCurrentWritePtr(), args...);
			getBatch().push(size);
		}

		static void vert_monochromeMix(const GL::Texture* texture, const auto... args){
			getBatch().checkFlush(texture, size);
			World::vert_monochromeMix(getBatch().getCurrentWritePtr(), args...);
			getBatch().push(size);
		}

		static void vert_monochromeAll(const GL::Texture* texture, const auto... args){
			getBatch().checkFlush(texture, size);
			World::vert_monochromeAll(getBatch().getCurrentWritePtr(), args...);
			getBatch().push(size);
		}
	};

	struct OverlayPass{
		static constexpr auto batchPtr = &Core::BatchGroup::overlay;

		static constexpr int size = GL::VERT_LENGTH_OVERLAY;

		inline static float vertices[size]{};

		static Core::Batch& getBatch();

		static void vert(const GL::Texture* texture, const auto... args){
			getBatch().checkFlush(texture, size);
			Overlay::vert(getBatch().getCurrentWritePtr(), args...);
			getBatch().push(size);
		}

		static void vert_monochromeMix(const GL::Texture* texture, const auto... args){
			getBatch().checkFlush(texture, size);
			Overlay::vert_monochromeMix(getBatch().getCurrentWritePtr(), args...);
			getBatch().push(size);
		}

		static void vert_monochromeAll(const GL::Texture* texture, const auto... args){
			getBatch().checkFlush(texture, size);
			Overlay::vert_monochromeAll(getBatch().getCurrentWritePtr(), args...);
			getBatch().push(size);
		}
	};
}


template <>
struct Graphic::Vertex::VertexPasser<&Core::BatchGroup::world> : Graphic::Vertex::WorldPass{};

template <>
struct Graphic::Vertex::VertexPasser<&Core::BatchGroup::overlay> : Graphic::Vertex::OverlayPass{};
