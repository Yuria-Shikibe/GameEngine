export module Game.Graphic.Draw;

import Game.Entity.Collision;

import Game.Chamber.FrameTrans;
import Game.Chamber.Frame;
import Game.Chamber;

import Core.BatchGroup;
import Core;

export import Graphic.Draw;
export import GL.Shader.UniformWrapper;
export import Assets.Graphic;

import ext.Guard;

namespace Game::Draw{
	namespace Colors = Graphic::Colors;
	namespace Draw = Graphic::Draw;

	export
	void hitbox(const ::Game::HitBox& hitBox){
		for(const auto& data : hitBox.hitBoxGroup){
			Draw::World::Fill::quad(Draw::World::getContextTexture(),
			                               data.original.v0, data.original.v1, data.original.v2, data.original.v3);
		}
	}

	export
	void hitbox(const ::Geom::QuadBox& hitBox){
		Draw::World::Fill::quad(Draw::World::getContextTexture(),
		                    hitBox.v0, hitBox.v1, hitBox.v2, hitBox.v3);
	}

	export
	template <typename Entity>
	void chamberFrameTile(const ChamberGridTrans<Entity>& chambers, Core::Renderer* renderer = Core::renderer,
	                      const bool disableDrawLimit = false){
		using namespace Graphic;
		using Draw::Overlay;

		[[maybe_unused]] ext::GuardRef guardRef1{Overlay::contextColor, Overlay::contextColor};
		[[maybe_unused]] ext::GuardRef guardRef2{Overlay::contextMixColor, Overlay::contextMixColor};
		[[maybe_unused]] Core::BatchGuard_L2W batchGuard{*Core::batchGroup.overlay, chambers.getLocalToWorld()};

		const float chamberTileAlpha = disableDrawLimit ? 1.0f : Math::curve(Core::camera->getScale(), 1.25f, 1.5f);

		if(chamberTileAlpha > 0.0f){
			if(renderer){
				renderer->frameBegin(renderer->effectBuffer);
				Overlay::color(Colors::GRAY, chamberTileAlpha);

				for(const auto& tile : chambers.getDrawable().invalids){
					Overlay::Fill::rectOrtho(Overlay::getContextTexture(), tile.get().getBound());
				}

				[[maybe_unused]] GL::UniformGuard guard_outline
					{
						Assets::Shaders::outlineArgs, 2.0f * Core::camera->getScale(), chambers.getLocalTrans().rot,
						renderer->getSize().inverse()
					};

				renderer->frameEnd(Assets::Shaders::outline_ortho);
			}

			Overlay::color(Colors::GRAY, 0.45f * chamberTileAlpha);
			Overlay::Line::setLineStroke(2.0f);
			for(const auto& tile : chambers.getDrawable().valids){
				Overlay::Line::rectOrtho(tile.get().getBound());
			}

			Overlay::color(Colors::LIGHT_GRAY, 0.85f * chamberTileAlpha);
			for(const auto tile : chambers.getDrawable().owners){
				Overlay::Line::rectOrtho(tile->getEntityBound());
			}



			Overlay::color(Colors::WHITE, chamberTileAlpha);
			Overlay::mixColor(Colors::BLACK.createLerp(Colors::RED_DUSK, 0.3f));

			[[maybe_unused]] GL::UniformGuard guard_slideLine_1{
					Assets::Shaders::slideLineShaderDrawArgs, 25.0f, 45.0f, Colors::CLEAR
				};
			[[maybe_unused]] Core::BatchGuard_Shader guard_batchShader{
					*Core::batchGroup.overlay, Assets::Shaders::sildeLines
				};


			for(const auto& tile : chambers.getDrawable().invalids){
				Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), tile.get().getBound());
			}
		}
	}

	export
	template <typename Entity>
	void chamberFrame(const Entity& entity, const ChamberGridTrans<Entity>& chambers,
	                  Core::Renderer* renderer = Core::renderer){
		using Graphic::Draw::Overlay;

		[[maybe_unused]] ext::GuardRef guardRef1{Overlay::contextColor, Overlay::contextColor};
		[[maybe_unused]] ext::GuardRef guardRef2{Overlay::contextMixColor, Overlay::contextMixColor};
		[[maybe_unused]] Core::BatchGuard_L2W batchGuard{*Core::batchGroup.world, chambers.getLocalToWorld()};

		for(const auto tile : chambers.getDrawable().owners){
			tile->draw(entity);
		}
	}

	// void
}
