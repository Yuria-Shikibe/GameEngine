//
// Created by Matrix on 2024/3/22.
//

export module Game.Graphic.Draw;

import Game.Entity.Collision;

import Game.Chamber.FrameTrans;
import Game.Chamber.Frame;
import Game.Chamber;

import Core.BatchGroup;

export import Graphic.Draw;
export import Core;
export import GL.Shader.UniformWrapper;
export import Assets.Graphic;

namespace Game::Draw{
	namespace Draw = Graphic::Draw;
	namespace Colors = Graphic::Colors;

	export void hitbox(const ::Game::HitBox& hitBox){
		for (const auto& data : hitBox.hitBoxGroup){

			Graphic::Draw::quad(Graphic::Draw::getDefaultTexture(),
				data.original.v0, data.original.v1, data.original.v2, data.original.v3);
		}
	}

	export void hitbox(const ::Geom::QuadBox& hitBox){
		Graphic::Draw::quad(Graphic::Draw::getDefaultTexture(),
			hitBox.v0, hitBox.v1, hitBox.v2, hitBox.v3);
	}

	export void chamberFrame(const ChamberFrameTrans& chambers, Core::Renderer* renderer = Core::renderer){
		[[maybe_unused]] auto guard = Draw::genColorGuard();
		[[maybe_unused]] Core::BatchGuard_L2W batchGuard{*Core::batchGroup.overlay, chambers.getTransformMat()};

		if(renderer){
			renderer->frameBegin(renderer->effectBuffer);
			Draw::color(Colors::GRAY);

			for(const Game::ChamberTile* tile : chambers.getDrawable().invalids){
				Draw::rectOrtho(Draw::globalState.defaultTexture, tile->getTileBound());
			}

			[[maybe_unused]] GL::UniformGuard guard_outline
				{Assets::Shaders::outlineArgs, 2.0f * Core::camera->getScale(), chambers.getLocalTrans().rot, renderer->getSize().inverse()};

			renderer->frameEnd(Assets::Shaders::outline_ortho);
		}
		// [[maybe_unused]] auto guard = Draw::genColorGuard();


		Draw::color(Colors::GRAY, 0.45f);
		Draw::Line::setLineStroke(2.0f);
		for(const auto* tile : chambers.getDrawable().valids){
			Draw::Line::rectOrtho(tile->getTileBound());
		}

		Draw::color(Colors::LIGHT_GRAY, 0.85f);
		for(const Game::Chamber* tile : chambers.getDrawable().owners){
			Draw::Line::rectOrtho(tile->getChamberBound());
		}

		Draw::color(Colors::WHITE);
		Draw::mixColor(Colors::BLACK.createLerp(Colors::RED_DUSK, 0.3f));

		[[maybe_unused]] GL::UniformGuard guard_slideLine_1{Assets::Shaders::slideLineShaderDrawArgs, 25.0f, 45.0f, Colors::CLEAR};
		// [[maybe_unused]] GL::UniformGuard guard_slideLine_2{Assets::Shaders::slideLineShaderAngle, chambers.getLocalTrans().rot + 45};
		// [[maybe_unused]] GL::UniformGuard guard_slideLine_3{Assets::Shaders::slideLineShaderScaleArgs, ~Core::renderer->getSize()};

		[[maybe_unused]] Core::BatchGuard_Shader guard_batchShader{*Core::batchGroup.overlay,  Assets::Shaders::sildeLines};

		for(const auto* tile : chambers.getDrawable().invalids){
			Draw::rectOrtho(Draw::getDefaultTexture(), tile->getTileBound());
		}
	}
	// void
}


