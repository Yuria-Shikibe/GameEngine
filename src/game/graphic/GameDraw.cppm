//
// Created by Matrix on 2024/3/22.
//

export module Game.Graphic.Draw;

import Game.Entity.Collision;
export import Graphic.Draw;

export namespace Graphic::Draw::Game{
	void hitbox(const ::Game::HitBox& hitBox){
		for (const auto& data : hitBox.hitBoxGroup){

			Graphic::Draw::quad(Graphic::Draw::defaultTexture,
				data.original.v0, data.original.v1, data.original.v2, data.original.v3);
		}
	}

	void hitbox(const ::Game::QuadBox& hitBox){
		Graphic::Draw::quad(Graphic::Draw::defaultTexture,
			hitBox.v0, hitBox.v1, hitBox.v2, hitBox.v3);
	}
}


