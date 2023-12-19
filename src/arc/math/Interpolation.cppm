module;

export module Interpolation;

import <functional>;
import Math;

export namespace Math::Interp{
	struct Pow{
		const float power;

		[[nodiscard]] explicit Pow(const float power)
			: power(power) {
		}

		template <typename T>
		[[nodiscard]] explicit Pow(const T& power)
			: power(static_cast<float>(power)) {
		}

		float operator()(const float a) const {
		    if(a <= 0.5f) return std::powf(a * 2, power) * 0.5f;
		    return std::powf((a - 1.0f) * 2.0f, power) / (std::fmod(power, 2.0f) == 0.0f ? -2.0f : 2.0f) + 1;
		}
     };

	struct PowIn : Pow{
		[[nodiscard]] explicit PowIn(const float power)
			: Pow(power) {
		}

		float operator()(const float a) const{
	        return std::powf(a, power);
	    }
	};
	
	struct PowOut : Pow{
		[[nodiscard]] explicit PowOut(const float power)
			: Pow(power) {
		}

		float operator()(const float a) const{
	        return std::powf(a - 1.0f, power) * (std::fmod(power, 2.0f) == 0.0f ? -1.0f : 1.0f) + 1.0f;
	    }
	};
}


export namespace Math::Interp{
    using InterpFunc = const std::function<float(float)>;

	InterpFunc linear = [](const float x){
		return x;
	};

	InterpFunc reverse = [](const float x){
		return 1.0f - x;
	};

	InterpFunc smooth = [](const float x){
		return  x * x * (3.0f - 2.0f * x);
	};

	InterpFunc smooth2 = [](float a){
		a = a * a * (3.0f - 2.0f * a);
		return a * a * (3.0f - 2.0f * a);
	};

	InterpFunc one = [](float a){
		return 1.0f;
	};

	InterpFunc zero = [](float a){
		return 1.0f;
	};

	InterpFunc slope = [](const float a){
		return 1.0f - std::abs(a - 0.5f) * 2.0f;
	};

	InterpFunc smoother = [](const float a){
		return a * a * a * (a * (a * 6.0f - 15.0f) + 10.0f);
	};

	InterpFunc& fade = smoother;
	
    InterpFunc pow2 = Pow(2);
    /** Slow, then fast. */
    InterpFunc pow2In = PowIn(2);
    InterpFunc& slowFast = pow2In;
    /** Fast, then slow. */
    InterpFunc pow2Out = PowOut(2);
    InterpFunc& fastSlow = pow2Out;
	InterpFunc pow2InInverse = [](const float a){
		return std::sqrtf(a);
	};

	InterpFunc pow2OutInverse = [](const float a){
		return std::sqrtf(-a + 1.0f);
	};

    InterpFunc pow3 = Pow(3);
    InterpFunc pow3In = PowIn(3);
    InterpFunc pow3Out = PowOut(3);
	InterpFunc pow3InInverse = [](const float a){
		return std::cbrtf(a);
	};
	InterpFunc pow3OutInverse = [](const float a){
		return std::cbrtf(1.0f - a);
	};
    InterpFunc pow4 = Pow(4);
    InterpFunc pow4In = PowIn(4);
    InterpFunc pow4Out = PowOut(4);
    InterpFunc pow5 = Pow(5);
    InterpFunc pow5In = PowIn(5);
    InterpFunc pow10In = PowIn(10);
    InterpFunc pow10Out = PowOut(10);
    InterpFunc pow5Out = PowOut(5);
	InterpFunc sine = [](const float a){
		return (1.0f - Math::cos(a * Math::PI)) * 0.5f;
	};
	InterpFunc sineIn = [](const float a){
		return 1.0f - Math::cos(a * Math::PI * 0.5f);
	};
	InterpFunc sineOut = [](const float a){
		return Math::sin(a * Math::PI * 0.5f);
	};
    //     Exp exp10 = Exp(2, 10);
    //     ExpIn exp10In = ExpIn(2, 10);
    //     ExpOut exp10Out = ExpOut(2, 10);
    //     Exp exp5 = Exp(2, 5);
    //     ExpIn exp5In = ExpIn(2, 5);
    //     ExpOut exp5Out = ExpOut(2, 5);
    //     Interp circle = a -> {
    //         if(a <= 0.5f){
    //             a *= 2;
    //             return (1 - (float)Math.sqrt(1 - a * a)) / 2;
    //         }
    //         a--;
    //         a *= 2;
    //         return ((float)Math.sqrt(1 - a * a) + 1) / 2;
    //     };
    //     Interp circleIn = a -> 1 - (float)Math.sqrt(1 - a * a);
    //     Interp circleOut = a -> {
    //         a--;
    //         return (float)Math.sqrt(1 - a * a);
    //     };
    //     Elastic elastic = Elastic(2, 10, 7, 1);
    //     ElasticIn elasticIn = ElasticIn(2, 10, 6, 1);
    //     ElasticOut elasticOut = ElasticOut(2, 10, 7, 1);
    //     Swing swing = Swing(1.5f);
    //     SwingIn swingIn = SwingIn(2f);
    //     SwingOut swingOut = SwingOut(2f);
    //     Bounce bounce = Bounce(4);
    //     BounceIn bounceIn = BounceIn(4);
    //     BounceOut bounceOut = BounceOut(4);
    //
    //     /** @param a Alpha value between 0 and 1. */
    //     float apply(float a);
    //
    //     /** @param a Alpha value between 0 and 1. */
    //     default float apply(float start, float end, float a){
    //         return start + (end - start) * apply(a);
    //     }
    //
    //
    //     class Exp implements Interp{
    //         final float value, power, min, scale;
    //
    //         Exp(float value, float power){
    //             this.value = value;
    //             this.power = power;
    //             min = (float)Math.pow(value, -power);
    //             scale = 1 / (1 - min);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             if(a <= 0.5f) return ((float)Math.pow(value, power * (a * 2 - 1)) - min) * scale / 2;
    //             return (2 - ((float)Math.pow(value, -power * (a * 2 - 1)) - min) * scale) / 2;
    //         }
    //     }
    //
    //     class ExpIn extends Exp{
    //         ExpIn(float value, float power){
    //             super(value, power);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             return ((float)Math.pow(value, power * (a - 1)) - min) * scale;
    //         }
    //     }
    //
    //     class ExpOut extends Exp{
    //         ExpOut(float value, float power){
    //             super(value, power);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             return 1 - ((float)Math.pow(value, -power * a) - min) * scale;
    //         }
    //     }
    //
    //     class Elastic implements Interp{
    //         final float value, power, scale, bounces;
    //
    //         Elastic(float value, float power, int bounces, float scale){
    //             this.value = value;
    //             this.power = power;
    //             this.scale = scale;
    //             this.bounces = bounces * Mathf.PI * (bounces % 2 == 0 ? 1 : -1);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             if(a <= 0.5f){
    //                 a *= 2;
    //                 return (float)Math.pow(value, power * (a - 1)) * Mathf.sin(a * bounces) * scale / 2;
    //             }
    //             a = 1 - a;
    //             a *= 2;
    //             return 1 - (float)Math.pow(value, power * (a - 1)) * Mathf.sin((a) * bounces) * scale / 2;
    //         }
    //     }
    //
    //     class ElasticIn extends Elastic{
    //         ElasticIn(float value, float power, int bounces, float scale){
    //             super(value, power, bounces, scale);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             if(a >= 0.99) return 1;
    //             return (float)Math.pow(value, power * (a - 1)) * Mathf.sin(a * bounces) * scale;
    //         }
    //     }
    //
    //     class ElasticOut extends Elastic{
    //         ElasticOut(float value, float power, int bounces, float scale){
    //             super(value, power, bounces, scale);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             if(a == 0) return 0;
    //             a = 1 - a;
    //             return (1 - (float)Math.pow(value, power * (a - 1)) * Mathf.sin(a * bounces) * scale);
    //         }
    //     }
    //
    //     class Bounce extends BounceOut{
    //         Bounce(float[] widths, float[] heights){
    //             super(widths, heights);
    //         }
    //
    //         Bounce(int bounces){
    //             super(bounces);
    //         }
    //
    //         private float out(float a){
    //             float test = a + widths[0] / 2;
    //             if(test < widths[0]) return test / (widths[0] / 2) - 1;
    //             return super.apply(a);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             if(a <= 0.5f) return (1 - out(1 - a * 2)) / 2;
    //             return out(a * 2 - 1) / 2 + 0.5f;
    //         }
    //     }
    //
    //     class BounceOut implements Interp{
    //         final float[] widths, heights;
    //
    //         BounceOut(float[] widths, float[] heights){
    //             if(widths.length != heights.length)
    //                 throw IllegalArgumentException("Must be the same number of widths and heights.");
    //             this.widths = widths;
    //             this.heights = heights;
    //         }
    //
    //         BounceOut(int bounces){
    //             if(bounces < 2 || bounces > 5)
    //                 throw IllegalArgumentException("bounces cannot be < 2 or > 5: " + bounces);
    //             widths = float[bounces];
    //             heights = float[bounces];
    //             heights[0] = 1;
    //             switch(bounces){
    //                 case 2:
    //                     widths[0] = 0.6f;
    //                     widths[1] = 0.4f;
    //                     heights[1] = 0.33f;
    //                     break;
    //                 case 3:
    //                     widths[0] = 0.4f;
    //                     widths[1] = 0.4f;
    //                     widths[2] = 0.2f;
    //                     heights[1] = 0.33f;
    //                     heights[2] = 0.1f;
    //                     break;
    //                 case 4:
    //                     widths[0] = 0.34f;
    //                     widths[1] = 0.34f;
    //                     widths[2] = 0.2f;
    //                     widths[3] = 0.15f;
    //                     heights[1] = 0.26f;
    //                     heights[2] = 0.11f;
    //                     heights[3] = 0.03f;
    //                     break;
    //                 case 5:
    //                     widths[0] = 0.3f;
    //                     widths[1] = 0.3f;
    //                     widths[2] = 0.2f;
    //                     widths[3] = 0.1f;
    //                     widths[4] = 0.1f;
    //                     heights[1] = 0.45f;
    //                     heights[2] = 0.3f;
    //                     heights[3] = 0.15f;
    //                     heights[4] = 0.06f;
    //                     break;
    //             }
    //             widths[0] *= 2;
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             if(a == 1) return 1;
    //             a += widths[0] / 2;
    //             float width = 0, height = 0;
    //             for(int i = 0, n = widths.length; i < n; i++){
    //                 width = widths[i];
    //                 if(a <= width){
    //                     height = heights[i];
    //                     break;
    //                 }
    //                 a -= width;
    //             }
    //             a /= width;
    //             float z = 4 / width * height * a;
    //             return 1 - (z - z * a) * width;
    //         }
    //     }
    //
    //     class BounceIn extends BounceOut{
    //         BounceIn(float[] widths, float[] heights){
    //             super(widths, heights);
    //         }
    //
    //         BounceIn(int bounces){
    //             super(bounces);
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             return 1 - super.apply(1 - a);
    //         }
    //     }
    //
    //     class Swing implements Interp{
    //         private final float scale;
    //
    //         Swing(float scale){
    //             this.scale = scale * 2;
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             if(a <= 0.5f){
    //                 a *= 2;
    //                 return a * a * ((scale + 1) * a - scale) / 2;
    //             }
    //             a--;
    //             a *= 2;
    //             return a * a * ((scale + 1) * a + scale) / 2 + 1;
    //         }
    //     }
    //
    //     class SwingOut implements Interp{
    //         private final float scale;
    //
    //         SwingOut(float scale){
    //             this.scale = scale;
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             a--;
    //             return a * a * ((scale + 1) * a + scale) + 1;
    //         }
    //     }
    //
    //     class SwingIn implements Interp{
    //         private final float scale;
    //
    //         SwingIn(float scale){
    //             this.scale = scale;
    //         }
    //
    //         @Override
    //         float apply(float a){
    //             return a * a * ((scale + 1) * a - scale);
    //         }
    //     }

}
