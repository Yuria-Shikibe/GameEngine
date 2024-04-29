//
// Created by Matrix on 2024/4/28.
//

export module Game.Chamber.FrameTrans;

export import Game.Chamber;
export import Game.ChamberFrame;
import GL.Buffer.FrameBuffer;
import Geom.Transform;
import Geom.Matrix3D;
import Geom.Shape.RectBox;
import Geom.Rect_Orthogonal;
import Core;

import std;

export namespace Game{
	/**
	 * @brief This frame should be workable on a async thread
	 */
	class ChamberFrameTrans{
		static constexpr float ExtendSize{100};

		Geom::Transform localTrans{};
		ChamberFrame frameData{};
		ChamberFrame::TileBrief drawable{};

		/** @brief Local draw usage*/
		Geom::Matrix3D transMat{};
		Geom::OrthoRectFloat frameBound{};
		Geom::QuadBox lastViewport{};

	public:
		void updateChamberFrameData(){
			frameBound = frameData.getBound();
		}

		[[nodiscard]] const Geom::Transform& getLocalTrans() const{ return localTrans; }

		void setLocalTrans(const Geom::Transform localTrans){
			this->localTrans = localTrans;
			// transMat.setScaling(0.05f, 0.05f).translateTo(localTrans);
			transMat.setTranslation(localTrans);
		}

		[[nodiscard]] Geom::Vec2 getWorldToLocal(const Geom::Vec2 worldPos) const noexcept{
			return (worldPos - getLocalTrans().vec).rotate(-localTrans.rot);
		}

		[[nodiscard]] const Geom::Matrix3D& getTransformMat() const noexcept{return transMat;}

		[[nodiscard]] ChamberFrame& getChambers(){ return frameData; }

		[[nodiscard]] ChamberFrame::TileBrief& getDrawable(){ return drawable; }

		Geom::QuadBox transformViewport(Geom::OrthoRectFloat viewport) const noexcept{
			viewport.expand(ExtendSize / 2, ExtendSize / 2);
			Geom::QuadBox portBox{viewport};
			Geom::Matrix3D mat{transMat};
			mat.inv(); //Get world to local

			for(unsigned i = 0; i < 4; ++i){
				portBox.vertAt(i) *= mat;
			}

			return portBox;
		}

		void updateDrawTarget(const Geom::OrthoRectFloat rawViewport, const bool nolimit = false){
			auto viewport = transformViewport(rawViewport);

			if(viewport == lastViewport && frameData.isBriefValid())return;
			viewport.updateBound();

			lastViewport = viewport;

			drawable.reset();

			if(frameData.getData().empty())return;
			frameBound = {};

			frameData.getQuadTree().intersectRegion(lastViewport, [](const auto& rect, const auto& quad){
				return quad.overlapRough(rect) && quad.overlapExact(rect);
			}, [this](const ChamberTile* tile, const Geom::QuadBox& view){
				if(tile->ownsChamber()){
					drawable.owners.push_back(tile->chamber.get());
				}

				if(tile->valid()){
					drawable.valids.push_back(tile);
				}else{
					drawable.invalids.push_back(tile);
				}
			});
			drawable.dataValid = true;
		}

		void update(const float delta){}

		[[nodiscard]] constexpr const Geom::OrthoRectFloat& getFrameBound() const noexcept { return frameBound; }

		[[nodiscard]] constexpr const auto& getLastViewport() const noexcept { return lastViewport; }
	};
}
