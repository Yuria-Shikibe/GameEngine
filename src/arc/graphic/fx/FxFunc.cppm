// ReSharper disable CppDFAConstantConditions
export module Graphic.FxFunc;

import ext.Concepts;
export import Math.Rand;
export import Math;
export import Math.Interpolation;
export import Geom.Transform;

export namespace Graphic{
	using SeedType = Math::Rand::SeedType;

	namespace FxGlobal{
		Geom::Vec2 vec2_0{};
		Geom::Vec2 vec2_1{};
		Geom::Vec2 vec2_2{};
		Geom::Vec2 vec2_3{};
		Geom::Vec2 vec2_4{};
		Geom::Vec2 vec2_5{};

		Math::Rand rand0{};
		Math::Rand rand1{};
		Math::Rand rand2{};
	}

	struct FxParam{
		const int count{0};
		const float progress{0};
		const float direction{0};
		const float toleranceAngle{180};
		const Math::Range radius{};
	};

	void splashVec(
		const SeedType seed,
		const FxParam& param,
		Concepts::Invokable<void(Geom::Vec2, Math::Rand&)> auto&& pred){

		Math::Rand rand{seed};
		Geom::Vec2 curVec{};

		for(int i = 0; i < param.count; ++i){
			curVec.setPolar(
				rand.random(param.direction - param.toleranceAngle, param.direction + param.toleranceAngle),
				param.radius.from + rand.random(param.radius.to - param.radius.from) * param.progress);

			pred(curVec, rand);
		}
	}

	void splashOut(const SeedType seed, const float subScale, const FxParam& param, Concepts::Invokable<void(Math::Rand&, Geom::Transform, float)> auto&& pred){
		Math::Rand rand{seed};
		Math::Rand rand2{};
		Geom::Vec2 vec2{};

		for(int current = 0; current < param.count; current++){
			const float sBegin = rand.random(1 - subScale);
			float
				fin = (param.progress -  sBegin) / subScale;

			if(fin < 0 || fin > 1)continue;

			rand2.setSeed(seed + current);
			const float theta = rand2.random(param.direction - param.toleranceAngle, param.direction + param.toleranceAngle);
			vec2.setPolar(theta, param.radius.from + rand.random(param.radius.to - param.radius.from) * fin);


			pred(rand2, Geom::Transform{vec2, theta}, fin);
		}
	}
}
