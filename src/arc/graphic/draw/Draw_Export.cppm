//
// Created by Matrix on 2024/5/27.
//

export module Graphic.Draw:Export;

import :Base;
import :Vertex;

namespace Graphic::Draw{
	struct OverlayBase : VertPostBase<OverlayBase>, HasBatch{
		static Core::Batch& getBatch(){
			return Vertex::VertexPasser<&Core::BatchGroup::overlay>::getBatch();
		}

		static void vert(
			const GL::Texture* texture,
			const float x1, const float y1, const float u1, const float v1, const Color c1, const Color cm1,
			const float x2, const float y2, const float u2, const float v2, const Color c2, const Color cm2,
			const float x3, const float y3, const float u3, const float v3, const Color c3, const Color cm3,
			const float x4, const float y4, const float u4, const float v4, const Color c4, const Color cm4
		){
			Vertex::VertexPasser<&Core::BatchGroup::overlay>::vert(texture,
			                                     x1, y1, u1, v1, c1, cm1, x2, y2, u2, v2, c2, cm2,
			                                     x3, y3, u3, v3, c3, cm3, x4, y4, u4, v4, c4, cm4
			);
		}

		static void vert_monochromeMix(
			const GL::Texture* texture, const Color cm,
			const float x1, const float y1, const float u1, const float v1, const Color c1,
			const float x2, const float y2, const float u2, const float v2, const Color c2,
			const float x3, const float y3, const float u3, const float v3, const Color c3,
			const float x4, const float y4, const float u4, const float v4, const Color c4
		){
			Vertex::VertexPasser<&Core::BatchGroup::overlay>::vert_monochromeMix(texture, cm,
			                                                   x1, y1, u1, v1, c1, x2, y2, u2, v2, c2,
			                                                   x3, y3, u3, v3, c3, x4, y4, u4, v4, c4
			);
		}

		static void vert_monochromeAll(
			const GL::Texture* texture, const Color cm, const Color c,
			const float x1, const float y1, const float u1, const float v1,
			const float x2, const float y2, const float u2, const float v2,
			const float x3, const float y3, const float u3, const float v3,
			const float x4, const float y4, const float u4, const float v4
		){
			Vertex::VertexPasser<&Core::BatchGroup::overlay>::vert_monochromeAll(texture, cm, c,
			                                                   x1, y1, u1, v1, x2, y2, u2, v2,
			                                                   x3, y3, u3, v3, x4, y4, u4, v4
			);
		}
	};
	struct WorldBase : VertPostBase<WorldBase>, HasBatch{
		static constexpr float DepthNear = 1;
		static constexpr float DepthFar = 50;

		inline static const GL::TextureRegionRect* defaultLightTexture = nullptr;
		inline static const GL::TextureRegionRect* defaultSolidTexture = nullptr;

		static inline float contextNorZ;

		static float getNormalizedDepth(const float z);

		static void setZ(const float z){ contextNorZ = getNormalizedDepth(z); }

		static void setNorZ(const float z){ contextNorZ = z; }

		[[nodiscard]] static float getNorZ(){ return contextNorZ; }

		static Core::Batch& getBatch(){
			return Vertex::VertexPasser<&Core::BatchGroup::world>::getBatch();
		}

		static void vert(
			const GL::Texture* texture,
			const float x1, const float y1, const float u1, const float v1, const Color c1, const Color cm1,
			const float x2, const float y2, const float u2, const float v2, const Color c2, const Color cm2,
			const float x3, const float y3, const float u3, const float v3, const Color c3, const Color cm3,
			const float x4, const float y4, const float u4, const float v4, const Color c4, const Color cm4
		){
			Vertex::WorldPass::vert(texture,
			                                     x1, y1, contextNorZ, u1, v1, c1, cm1, x2, y2, contextNorZ, u2, v2, c2,
			                                     cm2,
			                                     x3, y3, contextNorZ, u3, v3, c3, cm3, x4, y4, contextNorZ, u4, v4, c4,
			                                     cm4
			);
		}

		static void vert_monochromeMix(
			const GL::Texture* texture, const Color cm,
			const float x1, const float y1, const float u1, const float v1, const Color c1,
			const float x2, const float y2, const float u2, const float v2, const Color c2,
			const float x3, const float y3, const float u3, const float v3, const Color c3,
			const float x4, const float y4, const float u4, const float v4, const Color c4
		){
			Vertex::VertexPasser<&Core::BatchGroup::world>::vert_monochromeMix(texture, cm,
			                                                   x1, y1, contextNorZ, u1, v1, c1, x2, y2, contextNorZ, u2,
			                                                   v2, c2,
			                                                   x3, y3, contextNorZ, u3, v3, c3, x4, y4, contextNorZ, u4,
			                                                   v4, c4
			);
		}

		static void vert_monochromeAll(
			const GL::Texture* texture, const Color cm, const Color c,
			const float x1, const float y1, const float u1, const float v1,
			const float x2, const float y2, const float u2, const float v2,
			const float x3, const float y3, const float u3, const float v3,
			const float x4, const float y4, const float u4, const float v4
		){
			Vertex::VertexPasser<&Core::BatchGroup::world>::vert_monochromeAll(texture, cm, c,
			                                                   x1, y1, contextNorZ, u1, v1, x2, y2, contextNorZ, u2, v2,
			                                                   x3, y3, contextNorZ, u3, v3, x4, y4, contextNorZ, u4, v4
			);
		}
	};

	export using Overlay = DrawBase<OverlayBase>;
	export using World = DrawBase<WorldBase>;
	export using Void = DrawBase<>;

}