export module Math.Interpolation;

import std;
import Math;
import ext.Concepts;

//TODO uses constexpr program to enhance the effiency
export namespace Math::Interp::Func{
	// template <std::floating_point T, std::floating_point ...ArgT>
	// [[nodiscard]] constexpr T linear_n(T progress, ArgT... args) noexcept{
	// 	Math::ler
	// }

	template <std::floating_point T, std::floating_point ...ArgT>
	[[nodiscard]] constexpr T cos_n(T progress, ArgT... args) noexcept{
		std::array values{args...};

		T result{0.0};
		for (unsigned i = 0; i < sizeof...(ArgT); ++i) {
			T weight = 0.5 * (1.0 - Math::cos(2.0 * Math::PI * progress / static_cast<T>(sizeof...(ArgT) - 1u)));
			result += values[i] * weight;
		}

		return result;
	}

	template <std::floating_point T, std::size_t size>
	[[nodiscard]] constexpr T cos_n(T progress, std::array<T, size> values) noexcept{
		T result{0.0};
		for (unsigned i = 0; i < values.size(); ++i) {
			T weight = 0.5 * (1.0 - Math::cos(2.0 * Math::PI * progress / static_cast<T>(values.size() - 1u)));
			result += values[i] * weight;
		}

		return result;
	}

	template <float power>
	struct Pow{
		float operator()(const float a) const{
			if(a <= 0.5f) return std::powf(a * 2, power) * 0.5f;
			return std::powf((a - 1.0f) * 2.0f, power) / (std::fmod(power, 2.0f) == 0.0f ? -2.0f : 2.0f) + 1;
		}
	};


	template <auto Func, float powBegin = 0.5f, float minVal = 0.02f>
		requires Concepts::Invokable<decltype(Func), float(float)>
	struct LinePow{
		float operator()(const float a) const{
			auto t = a < powBegin ? minVal * a / powBegin : (1 - minVal) * Func((a - powBegin) / (1 - powBegin));

			return t;
		}
	};


	template <float power>
	struct PowIn{
		float operator()(const float a) const{
			return std::powf(a, power);
		}
	};


	template <float power>
	struct PowOut{
		float operator()(const float a) const{
			return std::powf(a - 1.0f, power) * (std::fmod(power, 2.0f) == 0.0f ? -1.0f : 1.0f) + 1.0f;
		}
	};


	struct Exp{
		const float value{}, power{}, min{}, scale{};

		Exp(const float value, const float power) : value(value),
		                                            power(power),
		                                            min{std::powf(value, -power)},
		                                            //Uses template param when powf is constexpr
		                                            scale(-1.0f / (1.0f - min)){}

		float operator()(const float a) const{
			if(a <= 0.5f) return (std::powf(value, power * (a * 2 - 1)) - min) * scale / 2;
			return (2 - (std::powf(value, -power * (a * 2 - 1)) - min) * scale) / 2;
		}
	};


	struct ExpIn : Exp{
		[[nodiscard]] ExpIn(const float value, const float power)
			: Exp(value, power){}

		float operator()(const float a) const{
			return (std::powf(value, power * (a - 1)) - min) * scale;
		}
	};


	struct ExpOut : Exp{
		[[nodiscard]] ExpOut(const float value, const float power)
			: Exp(value, power){}

		float operator()(const float a) const{
			return 1 - (std::powf(value, -power * a) - min) * scale;
		}
	};


	template <float value, float power, float scale, int bounces>
	struct Elastic{
		static constexpr float RealBounces = bounces * std::numbers::pi_v<float> * (bounces % 2 == 0 ? 1 : -1);

		float operator()(float a) const{
			if(a <= 0.5f){
				a *= 2;
				return std::powf(value, power * (a - 1)) * sin(a * RealBounces) * scale / 2;
			}
			a = 1 - a;
			a *= 2;
			return 1 - std::powf(value, power * (a - 1)) * sin(a * RealBounces) * scale / 2;
		}
	};


	template <float value, float power, float scale, int bounces>
	struct ElasticIn : Elastic<value, power, scale, bounces>{
		using Elastic<value, power, scale, bounces>::RealBounces;

		float operator()(const float a) const{
			if(a >= 0.99) return 1;
			return std::powf(value, power * (a - 1)) * sin(a * RealBounces) * scale;
		}
	};


	template <float value, float power, float scale, int bounces>
	struct ElasticOut : Elastic<value, power, scale, bounces>{
		using Elastic<value, power, scale, bounces>::RealBounces;

		float operator()(float a) const{
			if(a == 0) return 0;
			a = 1 - a;
			return 1 - std::powf(value, power * (a - 1)) * sin(a * RealBounces) * scale;
		}
	};


	template <size_t bounces>
	struct BounceOut{
		using Bounce = std::array<float, bounces>;
		const Bounce widths{}, heights{};

		[[nodiscard]] constexpr BounceOut(Bounce widths, Bounce heights) : widths(widths), heights(heights){}

		[[nodiscard]] constexpr BounceOut(){
			static_assert(bounces >= 2 && bounces <= 5, "bounces cannot be < 2 or > 5");

			auto& widths = const_cast<Bounce&>(this->widths);
			auto& heights = const_cast<Bounce&>(this->heights);

			heights[0] = 1;

			switch(bounces){
				case 2 : widths[0] = 0.6f;
					widths[1] = 0.4f;
					heights[1] = 0.33f;
					break;
				case 3 : widths[0] = 0.4f;
					widths[1] = 0.4f;
					widths[2] = 0.2f;
					heights[1] = 0.33f;
					heights[2] = 0.1f;
					break;
				case 4 : widths[0] = 0.34f;
					widths[1] = 0.34f;
					widths[2] = 0.2f;
					widths[3] = 0.15f;
					heights[1] = 0.26f;
					heights[2] = 0.11f;
					heights[3] = 0.03f;
					break;
				case 5 : widths[0] = 0.3f;
					widths[1] = 0.3f;
					widths[2] = 0.2f;
					widths[3] = 0.1f;
					widths[4] = 0.1f;
					heights[1] = 0.45f;
					heights[2] = 0.3f;
					heights[3] = 0.15f;
					heights[4] = 0.06f;
					break;
				default :;
			}

			widths[0] *= 2;
		}

		constexpr float operator()(float a) const{
			if(a == 1.0f) return 1;
			a += widths[0] / 2;
			float width = 0, height = 0;
			for(int i = 0, n = widths.size(); i < n; i++){
				width = widths[i];
				if(a <= width){
					height = heights[i];
					break;
				}
				a -= width;
			}
			a /= width;
			const float z = 4 / width * height * a;
			return 1 - (z - z * a) * width;
		}
	};


	template <size_t bounces>
	struct Bounce : BounceOut<bounces>{
		[[nodiscard]] constexpr Bounce(const typename BounceOut<bounces>::Bounce& widths,
		                               const typename BounceOut<bounces>::Bounce& heights)
			: BounceOut<bounces>(widths, heights){}

		[[nodiscard]] constexpr Bounce() = default;

	private:
		using BounceOut<bounces>::widths;

		constexpr float out(float a){
			float test = a + widths[0] / 2;
			if(test < widths[0]) return test / (widths[0] / 2) - 1;
			return BounceOut<bounces>::operator()(a);
		}

	public:
		constexpr float operator()(const float a){
			if(a <= 0.5f) return (1 - out(1 - a * 2)) / 2;
			return out(a * 2 - 1) / 2 + 0.5f;
		}
	};


	template <size_t bounces>
	struct BounceIn : BounceOut<bounces>{
		[[nodiscard]] constexpr BounceIn(const typename BounceOut<bounces>::Bounce& widths,
		                                 const typename BounceOut<bounces>::Bounce& heights)
			: BounceOut<bounces>(widths, heights){}

		[[nodiscard]] constexpr BounceIn() = default;

		constexpr float operator()(const float a){
			return 1 - BounceOut<bounces>::operator()(1 - a);
		}
	};


	template <float s>
	struct Swing{
		static constexpr float scale = s * 2.0f;

		constexpr float operator()(float a) const{
			if(a <= 0.5f){
				a *= 2;
				return a * a * ((scale + 1) * a - scale) / 2;
			}
			a -= 1.0f;
			a *= 2;
			return a * a * ((scale + 1) * a + scale) / 2 + 1;
		}
	};

	template <float s>
	struct SwingOut{
		static constexpr float scale = s;

		constexpr float operator()(float a) const{
			a -= 1.0f;
			return a * a * ((scale + 1) * a + scale) + 1;
		}
	};

	template <float s>
	struct SwingIn{
		static constexpr float scale = s;

		constexpr float operator()(const float a) const{
			return a * a * ((scale + 1) * a - scale);
		}
	};
}

export namespace Math::Interp{
	using InterpFunc = const std::function<float(float)>;

	constexpr auto linear = [](const float x){
		return x;
	};

	constexpr auto reverse = [](const float x){
		return 1.0f - x;
	};

	constexpr auto smooth = [](const float x){
		return x * x * (3.0f - 2.0f * x);
	};

	constexpr auto smooth2 = [](float a){
		a = a * a * (3.0f - 2.0f * a);
		return a * a * (3.0f - 2.0f * a);
	};

	constexpr auto one = [](float){
		return 1.0f;
	};

	constexpr auto zero = [](float){
		return 0.0f;
	};

	constexpr auto slope = [](const float a){
		return 1.0f - std::abs(a - 0.5f) * 2.0f;
	};

	constexpr auto smoother = [](const float a){
		return a * a * a * (a * (a * 6.0f - 15.0f) + 10.0f);
	};

	constexpr auto& fade = smoother;

	constexpr auto burst = Func::LinePow<Math::sqr, 0.925f>{};

	constexpr auto pow2 = Func::Pow<2>{};
	/** Slow, then fast. */
	constexpr auto pow2In = Func::PowIn<2>{};
	constexpr auto& slowFast = pow2In;
	/** Fast, then slow. */
	constexpr auto pow2Out = Func::PowOut<2>{};
	constexpr auto& fastSlow = pow2Out;
	constexpr auto pow2InInverse = [](const float a){
		return std::sqrtf(a);
	};

	constexpr auto pow2OutInverse = [](const float a){
		return std::sqrtf(-a + 1.0f);
	};

	constexpr auto pow3 = Func::Pow<3>{};
	constexpr auto pow3In = Func::PowIn<3>{};
	constexpr auto pow3Out = Func::PowOut<3>{};
	constexpr auto pow3InInverse = [](const float a){
		return std::cbrtf(a);
	};
	constexpr auto pow3OutInverse = [](const float a){
		return std::cbrtf(1.0f - a);
	};
	constexpr auto pow4 = Func::Pow<4>{};
	constexpr auto pow4In = Func::PowIn<4>{};
	constexpr auto pow4Out = Func::PowOut<4>{};
	constexpr auto pow5 = Func::Pow<5>{};
	constexpr auto pow5In = Func::PowIn<5>{};
	constexpr auto pow10In = Func::PowIn<10>{};
	constexpr auto pow10Out = Func::PowOut<10>{};
	constexpr auto pow5Out = Func::PowOut<5>{};
	constexpr auto sine = [](const float a){
		return (1.0f - Math::cos(a * Math::PI)) * 0.5f;
	};
	constexpr auto sineIn = [](const float a){
		return 1.0f - Math::cos(a * Math::PI * 0.5f);
	};
	constexpr auto sineOut = [](const float a){
		return Math::sin(a * Math::PI * 0.5f);
	};
	const auto exp10 = Func::Exp(2, 10);
	const auto exp10In = Func::ExpIn(2, 10);
	const auto exp10Out = Func::ExpOut(2, 10);
	const auto exp5 = Func::Exp(2, 5);
	const auto exp5In = Func::ExpIn(2, 5);
	const auto exp5Out = Func::ExpOut(2, 5);
	constexpr auto circle = [](float a){
		if(a <= 0.5f){
			a *= 2;
			return (1 - std::sqrtf(1 - a * a)) / 2;
		}
		a -= 1.0f;
		a *= 2;
		return (std::sqrtf(1 - a * a) + 1) / 2;
	};
	constexpr auto circleIn = [](const float a){
		return 1 - std::sqrtf(1 - a * a);
	};
	constexpr auto circleOut = [](float a){
		a -= 1.0f;
		return std::sqrtf(1 - a * a);
	};

	constexpr auto elastic = Func::Elastic<2, 10, 7, 1>{};
	constexpr auto elasticIn = Func::ElasticIn<2, 10, 6, 1>{};
	constexpr auto elasticOut = Func::ElasticOut<2, 10, 7, 1>{};
	constexpr auto swing = Func::Swing<1.5f>();
	constexpr auto swingIn = Func::SwingIn<2.0f>();
	constexpr auto swingOut = Func::SwingOut<2.0f>();
	constexpr auto bounce = Func::Bounce<4>();
	constexpr auto bounceIn = Func::BounceIn<4>();
	constexpr auto bounceOut = Func::BounceOut<4>();
}

export namespace Math::DiffApproach{
	// ret(src, tgt, delta)
	using DiffApproachFunc = float(float, float, float);
	using DiffApproachFuncPtr = std::decay_t<DiffApproachFunc>;

	// std::function<>

	DiffApproachFuncPtr ratioApproaching = Math::lerp;
	DiffApproachFuncPtr linearApproaching = Math::approach;
	DiffApproachFuncPtr instantApproaching = [](float, const float tgt, float){
		return tgt;
	};
}

export float operator |(const float val, Concepts::Invokable<float(float)> auto&& interp){
	return interp(val);
}
