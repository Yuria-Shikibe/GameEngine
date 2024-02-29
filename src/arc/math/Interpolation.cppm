module;

export module Interpolation;

import <functional>;
import Math;
import Concepts;
import <numbers>;

export namespace Math::Interp {
	struct Pow {
		const float power;

		[[nodiscard]] constexpr explicit Pow(const float power)
			: power(power) {
		}

		template <typename T>
		[[nodiscard]] constexpr explicit Pow(const T& power)
			: power(static_cast<float>(power)) {
		}

		float operator()(const float a) const {
			if(a <= 0.5f) return std::powf(a * 2, power) * 0.5f;
			return std::powf((a - 1.0f) * 2.0f, power) / (std::fmod(power, 2.0f) == 0.0f ? -2.0f : 2.0f) + 1;
		}
	};

	template <auto Func, float powBegin = 0.5f, float minVal = 0.02f>
		requires Concepts::Invokable<decltype(Func), float(float)>
	struct LinePow {
		float operator()(const float a) const {
			auto t = a < powBegin ? minVal * a / powBegin : (1 - minVal) * Func((a - powBegin) / (1 - powBegin));

			return t;
		}
	};

	struct PowIn : Pow {
		[[nodiscard]] constexpr explicit PowIn(const float power)
			: Pow(power) {
		}

		float operator()(const float a) const {
			return std::powf(a, power);
		}
	};

	struct PowOut : Pow {
		[[nodiscard]] constexpr explicit PowOut(const float power)
			: Pow(power) {
		}

		float operator()(const float a) const {
			return std::powf(a - 1.0f, power) * (std::fmod(power, 2.0f) == 0.0f ? -1.0f : 1.0f) + 1.0f;
		}
	};

	struct Exp {
		const float value{}, power{}, min{}, scale{};

		Exp(const float value, const float power) : value(value),
			power(power),
			min{ std::powf(value, -power) },
			scale(-1.0f / (1.0f - min)) {
		}

		float operator()(const float a) const {
			if(a <= 0.5f) return (std::powf(value, power * (a * 2 - 1)) - min) * scale / 2;
			return (2 - (std::powf(value, -power * (a * 2 - 1)) - min) * scale) / 2;
		}
	};

	struct ExpIn : Exp {
		[[nodiscard]] ExpIn(const float value, const float power)
			: Exp(value, power) {
		}

		float operator()(const float a) const {
			return (std::powf(value, power * (a - 1)) - min) * scale;
		}
	};

	struct ExpOut : Exp {
		[[nodiscard]] ExpOut(const float value, const float power)
			: Exp(value, power) {
		}

		float operator()(const float a) const {
			return 1 - (std::powf(value, -power * a) - min) * scale;
		}
	};

	struct Elastic {
		const float value{}, power{}, scale{}, bounces{};

		constexpr Elastic(const float value, const float power, const int bounces,
		                  const float scale) : value(value),
			power(power),
			scale(scale),
			bounces(bounces * std::numbers::pi_v<float> * (bounces % 2 == 0 ? 1 : -1)) {
		}

		float operator()(float a) const {
			if(a <= 0.5f) {
				a *= 2;
				return std::powf(value, power * (a - 1)) * sin(a * bounces) * scale / 2;
			}
			a = 1 - a;
			a *= 2;
			return 1 - std::powf(value, power * (a - 1)) * sin(a * bounces) * scale / 2;
		}
	};

	struct ElasticIn : Elastic {
		[[nodiscard]] constexpr ElasticIn(const float value, const float power, const int bounces,
		                                  const float scale)
			: Elastic(value, power, bounces, scale) {
		}

		float operator()(const float a) const {
			if(a >= 0.99) return 1;
			return std::powf(value, power * (a - 1)) * sin(a * bounces) * scale;
		}
	};

	struct ElasticOut : Elastic {
		[[nodiscard]] constexpr ElasticOut(const float value, const float power, const int bounces,
		                                   const float scale)
			: Elastic(value, power, bounces, scale) {
		}

		float operator()(float a) const {
			if(a == 0) return 0;
			a = 1 - a;
			return 1 - std::powf(value, power * (a - 1)) * sin(a * bounces) * scale;
		}
	};

	template <size_t bounces>
	struct BounceOut {
		using Bounce = std::array<float, bounces>;
		const Bounce widths{}, heights{};

		[[nodiscard]] constexpr BounceOut(Bounce widths, Bounce heights) : widths(widths), heights(heights) {
		}

		[[nodiscard]] constexpr BounceOut() {
			static_assert(bounces >= 2 && bounces <= 5, "bounces cannot be < 2 or > 5");

			Bounce& widths = const_cast<Bounce&>(this->widths);
			Bounce& heights = const_cast<Bounce&>(this->heights);

			heights[0] = 1;

			switch(bounces) {
				case 2 : widths[0] = 0.6f;
					widths[1]  = 0.4f;
					heights[1] = 0.33f;
					break;
				case 3 : widths[0] = 0.4f;
					widths[1]  = 0.4f;
					widths[2]  = 0.2f;
					heights[1] = 0.33f;
					heights[2] = 0.1f;
					break;
				case 4 : widths[0] = 0.34f;
					widths[1]  = 0.34f;
					widths[2]  = 0.2f;
					widths[3]  = 0.15f;
					heights[1] = 0.26f;
					heights[2] = 0.11f;
					heights[3] = 0.03f;
					break;
				case 5 : widths[0] = 0.3f;
					widths[1]  = 0.3f;
					widths[2]  = 0.2f;
					widths[3]  = 0.1f;
					widths[4]  = 0.1f;
					heights[1] = 0.45f;
					heights[2] = 0.3f;
					heights[3] = 0.15f;
					heights[4] = 0.06f;
					break;
				default :;
			}

			widths[0] *= 2;
		}

		constexpr float operator()(float a) const {
			if(a == 1.0f) return 1;
			a += widths[0] / 2;
			float width = 0, height = 0;
			for(int i = 0, n = widths.size(); i < n; i++) {
				width = widths[i];
				if(a <= width) {
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
	struct Bounce : BounceOut<bounces> {
		[[nodiscard]] constexpr Bounce(const typename BounceOut<bounces>::Bounce& widths,
		                               const typename BounceOut<bounces>::Bounce& heights)
			: BounceOut<bounces>(widths, heights) {
		}

		[[nodiscard]] constexpr Bounce() = default;

	private:
		using BounceOut<bounces>::widths;

		constexpr float out(float a) {
			float test = a + widths[0] / 2;
			if(test < widths[0]) return test / (widths[0] / 2) - 1;
			return BounceOut<bounces>::operator()(a);
		}

	public:
		constexpr float operator()(const float a) {
			if(a <= 0.5f) return (1 - out(1 - a * 2)) / 2;
			return out(a * 2 - 1) / 2 + 0.5f;
		}
	};

	template <size_t bounces>
	struct BounceIn : BounceOut<bounces> {
		[[nodiscard]] constexpr BounceIn(const typename BounceOut<bounces>::Bounce& widths,
		                                 const typename BounceOut<bounces>::Bounce& heights)
			: BounceOut<bounces>(widths, heights) {
		}

		[[nodiscard]] constexpr BounceIn() = default;

		constexpr float operator()(const float a) {
			return 1 - BounceOut<bounces>::operator()(1 - a);
		}
	};

	struct Swing {
		const float scale;

		[[nodiscard]] constexpr explicit Swing(const float scale) : scale(scale * 2) {
		}

		constexpr float operator()(float a) const {
			if(a <= 0.5f) {
				a *= 2;
				return a * a * ((scale + 1) * a - scale) / 2;
			}
			a -= 1.0f;
			a *= 2;
			return a * a * ((scale + 1) * a + scale) / 2 + 1;
		}
	};

	struct SwingOut {
		const float scale;

		[[nodiscard]] constexpr explicit SwingOut(const float scale)
			: scale(scale) {
		}

		constexpr float operator()(float a) const {
			a -= 1.0f;
			return a * a * ((scale + 1) * a + scale) + 1;
		}
	};

	struct SwingIn {
		const float scale;

		[[nodiscard]] constexpr explicit SwingIn(const float scale)
			: scale(scale) {
		}

		constexpr float operator()(const float a) const {
			return a * a * ((scale + 1) * a - scale);
		}
	};
}


export namespace Math::Interp {
	using InterpFunc = const std::function<float(float)>;

	InterpFunc linear = [](const float x) {
		return x;
	};

	InterpFunc reverse = [](const float x) {
		return 1.0f - x;
	};

	InterpFunc smooth = [](const float x) {
		return x * x * (3.0f - 2.0f * x);
	};

	InterpFunc smooth2 = [](float a) {
		a = a * a * (3.0f - 2.0f * a);
		return a * a * (3.0f - 2.0f * a);
	};

	InterpFunc one = [](float a) {
		return 1.0f;
	};

	InterpFunc zero = [](float a) {
		return 1.0f;
	};

	InterpFunc slope = [](const float a) {
		return 1.0f - std::abs(a - 0.5f) * 2.0f;
	};

	InterpFunc smoother = [](const float a) {
		return a * a * a * (a * (a * 6.0f - 15.0f) + 10.0f);
	};

	InterpFunc& fade = smoother;

	InterpFunc burst = LinePow<Math::sqr, 0.925f>{};

	InterpFunc pow2 = Pow(2);
	/** Slow, then fast. */
	InterpFunc pow2In    = PowIn(2);
	InterpFunc& slowFast = pow2In;
	/** Fast, then slow. */
	InterpFunc pow2Out       = PowOut(2);
	InterpFunc& fastSlow     = pow2Out;
	InterpFunc pow2InInverse = [](const float a) {
		return std::sqrtf(a);
	};

	InterpFunc pow2OutInverse = [](const float a) {
		return std::sqrtf(-a + 1.0f);
	};

	InterpFunc pow3          = Pow(3);
	InterpFunc pow3In        = PowIn(3);
	InterpFunc pow3Out       = PowOut(3);
	InterpFunc pow3InInverse = [](const float a) {
		return std::cbrtf(a);
	};
	InterpFunc pow3OutInverse = [](const float a) {
		return std::cbrtf(1.0f - a);
	};
	InterpFunc pow4     = Pow(4);
	InterpFunc pow4In   = PowIn(4);
	InterpFunc pow4Out  = PowOut(4);
	InterpFunc pow5     = Pow(5);
	InterpFunc pow5In   = PowIn(5);
	InterpFunc pow10In  = PowIn(10);
	InterpFunc pow10Out = PowOut(10);
	InterpFunc pow5Out  = PowOut(5);
	InterpFunc sine     = [](const float a) {
		return (1.0f - Math::cos(a * Math::PI)) * 0.5f;
	};
	InterpFunc sineIn = [](const float a) {
		return 1.0f - Math::cos(a * Math::PI * 0.5f);
	};
	InterpFunc sineOut = [](const float a) {
		return Math::sin(a * Math::PI * 0.5f);
	};
	InterpFunc exp10    = Exp(2, 10);
	InterpFunc exp10In  = ExpIn(2, 10);
	InterpFunc exp10Out = ExpOut(2, 10);
	InterpFunc exp5     = Exp(2, 5);
	InterpFunc exp5In   = ExpIn(2, 5);
	InterpFunc exp5Out  = ExpOut(2, 5);
	InterpFunc circle   = [](float a) {
		if(a <= 0.5f) {
			a *= 2;
			return (1 - std::sqrtf(1 - a * a)) / 2;
		}
		a -= 1.0f;
		a *= 2;
		return (std::sqrtf(1 - a * a) + 1) / 2;
	};
	InterpFunc circleIn = [](const float a) {
		return 1 - std::sqrtf(1 - a * a);
	};
	InterpFunc circleOut = [](float a) {
		a -= 1.0f;
		return std::sqrtf(1 - a * a);
	};

	InterpFunc elastic    = Elastic(2, 10, 7, 1);
	InterpFunc elasticIn  = ElasticIn(2, 10, 6, 1);
	InterpFunc elasticOut = ElasticOut(2, 10, 7, 1);
	InterpFunc swing      = Swing(1.5f);
	InterpFunc swingIn    = SwingIn(2.0f);
	InterpFunc swingOut   = SwingOut(2.0f);
	InterpFunc bounce     = Bounce<4>();
	InterpFunc bounceIn   = BounceIn<4>();
	InterpFunc bounceOut  = BounceOut<4>();
}
