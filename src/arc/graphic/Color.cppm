module ;

export module Graphic.Color;

import std;
import Math;
import Math.Bit;
import Geom.Vector3D;


export namespace Graphic{
	using std::max;
	using std::min;

	/**
	 * \brief  32Bits for 4 u byte[0, 255]
	 * \code
	 * 00000000__00000000__00000000__00000000
	 * r value^  g value^  b value^  a value^
	 *       24        16         8         0
	 * \endcode
	 */
	class Color{
	public:
		static constexpr auto maxVal           = std::numeric_limits<unsigned char>::max();
		static constexpr float maxValF         = std::numeric_limits<unsigned char>::max();
		static constexpr unsigned int r_Offset = 24;
		static constexpr unsigned int g_Offset = 16;
		static constexpr unsigned int b_Offset = 8 ;
		static constexpr unsigned int a_Offset = 0 ;
		static constexpr unsigned int a_Mask   = 0x00'00'00'ff;
		static constexpr unsigned int b_Mask   = 0x00'00'ff'00;
		static constexpr unsigned int g_Mask   = 0x00'ff'00'00;
		static constexpr unsigned int r_Mask   = 0xff'00'00'00;

		float r = 0, g = 0, b = 0, a = 0;

		using ColorBits = unsigned int;

		constexpr Color() = default;

		constexpr explicit Color(const ColorBits rgba8888V){
			rgba8888(rgba8888V);
		}

		constexpr explicit Color(const std::tuple<float, float, float, float>& datas){
			set(datas);
		}

		constexpr Color(const float r, const float g, const float b, const float a)
			: r(r),
			  g(g),
			  b(b),
			  a(a){
			clamp();
		}

		constexpr Color(const float r, const float g, const float b): Color(r, g, b, 1){}

		[[nodiscard]] const float* asRaw() const{
			return reinterpret_cast<const float*>(this);
		}

		constexpr Color& set(std::tuple<float, float, float, float> datas){
			std::tie(r, g, b, a) = datas;
			return  *this;
		}

		static constexpr size_t hash_value(const Color& obj){
			return obj.hashCode();
		}

		friend void swap(Color& lhs, Color& rhs) noexcept{
			using std::swap;
			swap(lhs.r, rhs.r);
			swap(lhs.g, rhs.g);
			swap(lhs.b, rhs.b);
			swap(lhs.a, rhs.a);
		}

		friend std::ostream& operator<<(std::ostream& os, const Color& obj){
			os << std::setw(8) << std::setfill('0') << std::hex << (static_cast<ColorBits>(255 * obj.r) << 24 | static_cast<ColorBits>(255 * obj.g) << 16 | static_cast<ColorBits>(255 * obj.b) << 8 | static_cast<ColorBits>(255 * obj.a));

			return os;
		}

		friend bool operator==(const Color& lhs, const Color& rhs){
			return lhs.r == rhs.r
				&& lhs.g == rhs.g
				&& lhs.b == rhs.b
				&& lhs.a == rhs.a;
		}

		friend bool operator!=(const Color& lhs, const Color& rhs){
			return !(lhs == rhs);
		}

		static constexpr Color valueOf(const std::string_view hex){
			const int offset = hex[0] == '#' ? 1 : 0;

			const int r = parseHex(hex, offset, offset + 2);
			const int g = parseHex(hex, offset + 2, offset + 4);
			const int b = parseHex(hex, offset + 4, offset + 6);
			const int a = hex.length() - offset != 8 ? 255 : parseHex(hex, offset + 6, offset + 8);
			return Color{static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f};
		}

		static constexpr Color& valueOf(Color& color, const std::string_view hex){
			const int offset = hex[0] == '#' ? 1 : 0;

			const int r = parseHex(hex, offset, offset + 2);
			const int g = parseHex(hex, offset + 2, offset + 4);
			const int b = parseHex(hex, offset + 4, offset + 6);
			const int a = hex.length() - offset != 8 ? 255 : parseHex(hex, offset + 6, offset + 8);
			return color.set(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f);
		}

		static constexpr int parseHex(const std::string_view string, const int from, const int to){
			int total = 0;
			for (int i = from; i < to; i++) {
				total += Math::charToDigitValue(string[i], 16) * (i == from ? 16 : 1);
			}
			return total;
		}

		static constexpr float toFloatBits(const int r, const int g, const int b, const int a){
			const int color = a << 24 | b << 16 | g << 8 | r;
			return intToFloatColor(color);
		}

		static constexpr float toFloatBits(const float r, const float g, const float b, const float a){
			const int color = static_cast<int>(255 * a) << 24 | static_cast<int>(255 * b) << 16 | static_cast<int>(255 * g) << 8 | static_cast<int>(255 * r);
			return intToFloatColor(color);
		}

		static constexpr double toDoubleBits(const float r, const float g, const float b, const float a){
			return std::bit_cast<double>(rgba8888(r, g, b, a) & Math::INT_MASK_BACK);
		}

		static constexpr double toDoubleBits(const int r, const int g, const int b, const int a){
			return toDoubleBits(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f); // NOLINT(*-narrowing-conversions)
		}

		static constexpr ColorBits abgr(const int tr, const int tg, const int tb, const int ta){
			return ta << 24 | tb << 16 | tg << 8 | tr;
		}

		static constexpr ColorBits alpha(const float alpha){
			return static_cast<ColorBits>(alpha * 255.0f);
		}

		static constexpr ColorBits luminanceAlpha(const float luminance, const float alpha){
			return static_cast<ColorBits>(luminance * 255.0f) << 8 | static_cast<ColorBits>(alpha * 255.0f);
		}

		static constexpr ColorBits rgb565(const float r, const float g, const float b){
			return static_cast<ColorBits>(r * 31) << 11 | static_cast<ColorBits>(g * 63) << 5 | static_cast<ColorBits>(b * 31);
		}

		static constexpr ColorBits rgba4444(const float r, const float g, const float b, const float a){
			return static_cast<ColorBits>(r * 15) << 12 | static_cast<ColorBits>(g * 15) << 8 | static_cast<ColorBits>(b * 15) << 4 | static_cast<ColorBits>(a * 15);
		}

		static constexpr ColorBits rgb888(const float r, const float g, const float b){
			return static_cast<ColorBits>(r * 255) << 16 | static_cast<ColorBits>(g * 255) << 8 | static_cast<ColorBits>(b * 255);
		}

		static constexpr ColorBits rgba8888(const float r, const float g, const float b, const float a){
			return static_cast<ColorBits>(r * 255) << 24 | static_cast<ColorBits>(g * 255) << 16 | static_cast<ColorBits>(b * 255) << 8 | static_cast<ColorBits>(a * 255);
		}

		static constexpr ColorBits argb8888(const float a, const float r, const float g, const float b){
			return static_cast<ColorBits>(a * 255) << 24 | static_cast<ColorBits>(r * 255) << 16 | static_cast<ColorBits>(g * 255) << 8 | static_cast<ColorBits>(b * 255);
		}

		static int packRgba(const int r, const int g, const int b, const int a){
			return r << 24 | g << 16 | b << 8 | a;
		}

		[[nodiscard]] constexpr ColorBits rgb565() const{
			return static_cast<ColorBits>(r * 31) << 11 | static_cast<ColorBits>(g * 63) << 5 | static_cast<ColorBits>(b * 31);
		}

		[[nodiscard]] constexpr ColorBits rgba4444() const{
			return static_cast<ColorBits>(r * 15) << 12 | static_cast<ColorBits>(g * 15) << 8 | static_cast<ColorBits>(b * 15) << 4 | static_cast<ColorBits>(a * 15);
		}

		[[nodiscard]] constexpr ColorBits rgb888() const{
			return static_cast<ColorBits>(r * 255) << 16 | static_cast<ColorBits>(g * 255) << 8 | static_cast<ColorBits>(b * 255);
		}

		[[nodiscard]] constexpr ColorBits rgba8888() const{
			return static_cast<ColorBits>(r * 255) << 24 | static_cast<ColorBits>(g * 255) << 16 | static_cast<ColorBits>(b * 255) << 8 | static_cast<ColorBits>(a * 255);
		}

		[[nodiscard]] constexpr ColorBits argb8888() const{
			return static_cast<ColorBits>(a * 255) << 24 | static_cast<ColorBits>(r * 255) << 16 | static_cast<ColorBits>(g * 255) << 8 | static_cast<ColorBits>(b * 255);
		}

		constexpr Color& rgb565(const ColorBits value){
			r = static_cast<float>((value & 0x0000F800) >> 11) / 31.0f;
			g = static_cast<float>((value & 0x000007E0) >> 5) / 63.0f;
			b = static_cast<float>((value & 0x0000001F)) / 31.0f;
			return *this;
		}

		constexpr Color& rgba4444(const ColorBits value){
			r = static_cast<float>((value & 0x0000f000) >> 12) / 15.0f;
			g = static_cast<float>((value & 0x00000f00) >> 8) / 15.0f;
			b = static_cast<float>((value & 0x000000f0) >> 4) / 15.0f;
			a = static_cast<float>((value & 0x0000000f)) / 15.0f;
			return *this;
		}

		constexpr Color& rgb888(const ColorBits value){
			r = static_cast<float>((value & 0x00ff0000) >> 16) / 255.0f;
			g = static_cast<float>((value & 0x0000ff00) >> 8) / 255.0f;
			b = static_cast<float>((value & 0x000000ff)) / 255.0f;
			return *this;
		}

		constexpr Color& rgba8888(const ColorBits value){
			r = static_cast<float>((value & 0xff000000) >> 24) / 255.0f;
			g = static_cast<float>((value & 0x00ff0000) >> 16) / 255.0f;
			b = static_cast<float>((value & 0x0000ff00) >> 8) / 255.0f;
			a = static_cast<float>((value & 0x000000ff)) / 255.0f;
			return *this;
		}

		constexpr Color& argb8888(const ColorBits value){
			a = static_cast<float>((value & 0xff000000) >> 24) / 255.0f;
			r = static_cast<float>((value & 0x00ff0000) >> 16) / 255.0f;
			g = static_cast<float>((value & 0x0000ff00) >> 8) / 255.0f;
			b = static_cast<float>((value & 0x000000ff)) / 255.0f;
			return *this;
		}

		constexpr Color& abgr8888(const float value){
			const unsigned int c = floatToIntColor(value);
			a = static_cast<float>((c & 0xff000000) >> 24) / 255.0f;
			b = static_cast<float>((c & 0x00ff0000) >> 16) / 255.0f;
			g = static_cast<float>((c & 0x0000ff00) >> 8) / 255.0f;
			r = static_cast<float>((c & 0x000000ff)) / 255.0f;
			return *this;
		}

		static constexpr Color grays(const float value){
			return Color{ value, value, value };
		}

		static constexpr Color rgb(const int r, const int g, const int b){
			return Color{ (static_cast<float>(r) / 255.0f), (static_cast<float>(g) / 255.0f), (static_cast<float>(b) / 255.0f) };
		}

		static constexpr unsigned floatToIntColor(const float value){
			auto intBits = std::bit_cast<ColorBits>(value);
			intBits |= static_cast<ColorBits>(static_cast<float>(intBits >> 24) * (255.0f / 254.0f)) << 24;
			return intBits;
		}

		static constexpr float intToFloatColor(const ColorBits value){
			return std::bit_cast<float>(value & 0xfeffffff);
		}

		[[nodiscard]] constexpr float diff(const Color& other) const {
			return Math::abs(hue() - other.hue()) / 360 + Math::abs(value() - other.value()) + Math::abs(saturation() - other.saturation());
		}

		[[nodiscard]] constexpr ColorBits rgba() const{
			return rgba8888();
		}

		constexpr Color& set(const Color& color){
			this->r = color.r;
			this->g = color.g;
			this->b = color.b;
			this->a = color.a;
			return *this;
		}

		constexpr Color& mul(const Color& color){
			this->r *= color.r;
			this->g *= color.g;
			this->b *= color.b;
			this->a *= color.a;
			return clamp();
		}

		constexpr Color& mul_rgb(const float value){
			this->r *= value;
			this->g *= value;
			this->b *= value;
			return clamp();
		}

		constexpr Color& mul_rgba(const float value){
			this->r *= value;
			this->g *= value;
			this->b *= value;
			this->a *= value;
			return clamp();
		}

		constexpr Color& add(const Color& color){
			this->r += color.r;
			this->g += color.g;
			this->b += color.b;
			return clamp();
		}

		constexpr Color& sub(const Color& color){
			this->r -= color.r;
			this->g -= color.g;
			this->b -= color.b;
			return clamp();
		}

		constexpr Color& clamp(){
			if (r < 0)
				r = 0;
			else if (r > 1) r = 1;

			if (g < 0)
				g = 0;
			else if (g > 1) g = 1;

			if (b < 0)
				b = 0;
			else if (b > 1) b = 1;

			if (a < 0)
				a = 0;
			else if (a > 1) a = 1;
			return *this;
		}

		constexpr Color& setForce(const float tr, const float tg, const float tb, const float ta){
			this->r = tr;
			this->g = tg;
			this->b = tb;
			this->a = ta;

			return *this;
		}

		constexpr Color& set(const float tr, const float tg, const float tb, const float ta){
			this->r = tr;
			this->g = tg;
			this->b = tb;
			this->a = ta;
			return clamp();
		}

		constexpr Color& set(const float tr, const float tg, const float tb){
			this->r = tr;
			this->g = tg;
			this->b = tb;
			return clamp();
		}

		constexpr Color& set(const int rgba){
			return rgba8888(rgba);
		}

		[[nodiscard]] constexpr float sum() const{
			return r + g + b;
		}

		constexpr Color& add(const float tr, const float tg, const float tb, const float ta){
			this->r += tr;
			this->g += tg;
			this->b += tb;
			this->a += ta;
			return clamp();
		}

		constexpr Color& add(const float tr, const float tg, const float tb){
			this->r += tr;
			this->g += tg;
			this->b += tb;
			return clamp();
		}

		constexpr Color& sub(const float tr, const float tg, const float tb, const float ta){
			this->r -= tr;
			this->g -= tg;
			this->b -= tb;
			this->a -= ta;
			return clamp();
		}

		constexpr Color& sub(const float tr, const float tg, const float tb){
			this->r -= tr;
			this->g -= tg;
			this->b -= tb;
			return clamp();
		}

		constexpr Color& inv(){
			r = 1 - r;
			g = 1 - g;
			b = 1 - b;
			return *this;
		}

		constexpr Color& setR(const float tr){
			this->r -= tr;
			return *this;
		}

		constexpr Color& setG(const float tg){
			this->g -= tg;
			return *this;
		}

		constexpr Color& setB(const float tb){
			this->b -= tb;
			return *this;
		}

		constexpr Color& setA(const float ta){
			this->a = ta;
			return *this;
		}

		constexpr Color& mulA(const float ta){
			this->a *= ta;
			return *this;
		}

		constexpr Color& mul(const float tr, const float tg, const float tb, const float ta){
			this->r *= tr;
			this->g *= tg;
			this->b *= tb;
			this->a *= ta;
			return clamp();
		}

		constexpr Color& mul(const float val){
			this->r *= val;
			this->g *= val;
			this->b *= val;
			this->a *= val;
			return clamp();
		}

		constexpr Color& lerp(const Color& target, const float t){
			this->r += t * (target.r - this->r);
			this->g += t * (target.g - this->g);
			this->b += t * (target.b - this->b);
			this->a += t * (target.a - this->a);
			return clamp();
		}

		constexpr Color& lerp(const float tr, const float tg, const float tb, const float ta, const float t){
			this->r += t * (tr - this->r);
			this->g += t * (tg - this->g);
			this->b += t * (tb - this->b);
			this->a += t * (ta - this->a);
			return clamp();
		}

		constexpr Color& preMultiplyAlpha(){
			r *= a;
			g *= a;
			b *= a;
			return *this;
		}

		constexpr Color& write(Color& to) const{
			return to.set(*this);
		}

		using HSVType = std::array<float, 3>;

		[[nodiscard]] constexpr float hue() const{
			return toHsv()[0];
		}

		[[nodiscard]] constexpr float saturation() const{
			return toHsv()[1];
		}

		[[nodiscard]] constexpr float value() const{
			return toHsv()[2];
		}

		Color& byHue(const float amount){
			HSVType TMP_HSV = toHsv();
			TMP_HSV[0] = amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& bySaturation(const float amount){
			HSVType TMP_HSV = toHsv();
			TMP_HSV[1] = amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& byValue(const float amount){
			HSVType TMP_HSV = toHsv();
			TMP_HSV[2] = amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& shiftHue(const float amount){
			HSVType TMP_HSV = toHsv();
			TMP_HSV[0] += amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& shiftSaturation(const float amount){
			HSVType TMP_HSV = toHsv();
			TMP_HSV[1] += amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& shiftValue(const float amount){
			HSVType TMP_HSV = toHsv();
			TMP_HSV[2] += amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		[[nodiscard]] constexpr size_t hashCode() const{
			return rgba8888();
		}

		[[nodiscard]] constexpr float toFloatBits() const{
			const ColorBits color = static_cast<ColorBits>(255 * a) << 24 | static_cast<ColorBits>(255 * b) << 16 | static_cast<ColorBits>(255 * g) << 8 | static_cast<ColorBits>(255 * r);
			return intToFloatColor(color);
		}

		[[nodiscard]] constexpr double toDoubleBits() const{
			return toDoubleBits(r, g, b, a); // NOLINT(*-narrowing-conversions)
		}

		[[nodiscard]] constexpr ColorBits abgr() const{
			return static_cast<ColorBits>(255 * a) << 24 | static_cast<ColorBits>(255 * b) << 16 | static_cast<ColorBits>(255 * g) << 8 | static_cast<ColorBits>(255 * r);
		}

		[[nodiscard]] std::string toString() const{
			std::stringstream ss;

			ss << *this;

			return ss.str();
		}

		Color& fromHsv(const float h, const float s, const float v){
			const float x = std::fmod(h / 60.0f + 6, static_cast<float>(6));
			const int i = static_cast<int>(x);
			const float f = x - static_cast<float>(i);
			const float p = v * (1 - s);
			const float q = v * (1 - s * f);
			const float t = v * (1 - s * (1 - f));
			switch (i) {
				case 0:
					r = v;
					g = t;
					b = p;
					break;
				case 1:
					r = q;
					g = v;
					b = p;
					break;
				case 2:
					r = p;
					g = v;
					b = t;
					break;
				case 3:
					r = p;
					g = q;
					b = v;
					break;
				case 4:
					r = t;
					g = p;
					b = v;
					break;
				default:
					r = v;
					g = p;
					b = q;
			}

			return clamp();
		}

		Color& fromHsv(const HSVType hsv){
			return fromHsv(hsv[0], hsv[1], hsv[2]);
		}

		constexpr Color HSVtoRGB(const float h, const float s, const float v, const float alpha){
			Color c = HSVtoRGB(h, s, v);
			c.a = alpha;
			return c;
		}

		constexpr Color HSVtoRGB(const float h, const float s, const float v){
			Color c{ 1, 1, 1, 1 };
			HSVtoRGB(h, s, v, c);
			return c;
		}

		[[nodiscard]] constexpr HSVType toHsv() const{
			HSVType hsv = {};

			const float maxV = max(max(r, g), b);
			const float minV = min(min(r, g), b);
			if (const float range = maxV - minV; range == 0) {
				hsv[0] = 0;
			}
			else if (maxV == r) {
				hsv[0] = fmod(60 * (g - b) / range + 360, 360.0f);
			}
			else if (maxV == g) {
				hsv[0] = 60 * (b - r) / range + 120;
			}
			else {
				hsv[0] = 60 * (r - g) / range + 240;
			}

			if (maxV > 0) {
				hsv[1] = 1 - minV / maxV;
			}
			else {
				hsv[1] = 0;
			}

			hsv[2] = maxV;

			return hsv;
		}

		constexpr Color& HSVtoRGB(float h, float s, float v, Color& targetColor){
			if (h == 360) h = 359;
			h = Math::max(0.0f, Math::min(360.0f, h));
			s = Math::max(0.0f, Math::min(100.0f, s));
			v = Math::max(0.0f, Math::min(100.0f, v));
			s /= 100.0f;
			v /= 100.0f;
			h /= 60.0f;
			const int i = Math::floor(h);
			const float f = h - static_cast<float>(i);
			const float p = v * (1 - s);
			const float q = v * (1 - s * f);
			const float t = v * (1 - s * (1 - f));
			switch (i) {
				case 0:
					r = v;
					g = t;
					b = p;
					break;
				case 1:
					r = q;
					g = v;
					b = p;
					break;
				case 2:
					r = p;
					g = v;
					b = t;
					break;
				case 3:
					r = p;
					g = q;
					b = v;
					break;
				case 4:
					r = t;
					g = p;
					b = v;
					break;
				default:
					r = v;
					g = p;
					b = q;
			}

			targetColor.setA(targetColor.a);
			return targetColor;
		}

		static constexpr int clampf(const float value){
			return Math::min(Math::max(static_cast<int>(value), 0), 255);
		}

		static constexpr ColorBits ri(const ColorBits rgba){
			return (rgba & 0xff000000) >> 24;
		}

		static constexpr ColorBits gi(const ColorBits rgba){
			return (rgba & 0x00ff0000) >> 16;
		}

		static constexpr ColorBits bi(const ColorBits rgba){
			return (rgba & 0x0000ff00) >> 8;
		}

		static constexpr ColorBits ai(const ColorBits rgba){
			return rgba & 0x000000ff;
		}

		static constexpr ColorBits muli(const ColorBits ca, const ColorBits cb){
			const ColorBits
					rV = (ca & r_Mask) >> r_Offset,
					gV = (ca & g_Mask) >> g_Offset,
					bV = (ca & b_Mask) >> b_Offset,
					aV = ca & a_Mask,
					r2 = (cb & r_Mask) >> r_Offset,
					g2 = (cb & g_Mask) >> g_Offset,
					b2 = (cb & b_Mask) >> b_Offset,
					a2 = cb & a_Mask;
			return clampf(static_cast<float>(rV * r2) / 255.0f) << 24 | clampf(static_cast<float>(gV * g2) / 255.0f) << 16 | clampf(static_cast<float>(bV * b2) / 255.0f) << 8 | clampf(static_cast<float>(aV * a2) / 255.0f);
		}

		static constexpr ColorBits muli(const ColorBits rgba, const float value){
			const ColorBits
					rV = (rgba & r_Mask) >> r_Offset,
					gV = (rgba & g_Mask) >> g_Offset,
					bV = (rgba & b_Mask) >> b_Offset,
					aV = rgba & a_Mask;
			return clampf(static_cast<float>(rV) * value) << 24 | clampf(static_cast<float>(gV) * value) << 16 | clampf(static_cast<float>(bV) * value) << 8 | aV;
		}

		/** @brief [r, g, b, a] */
		using ColorData = std::tuple<float, float, float, float>;

		[[nodiscard]] static constexpr ColorData getLerpVal(const float s, const auto&... colors) {
			constexpr size_t size = sizeof...(colors);
			constexpr size_t bound = size - 1;
			const std::array<Color, size> arr = {colors...};

			const Color& from = arr[Math::clamp(static_cast<size_t>(s * bound), 0ull, bound)];
			const Color& to = arr[Math::clamp(static_cast<size_t>(s * bound + 1), 0ull, bound)];

			const float toWeight = s * bound - static_cast<int>(s * bound);
			const float fromWeight = 1.0f - toWeight;
			return ColorData{ from.r * fromWeight + to.r * toWeight, from.g * fromWeight + to.g * toWeight, from.b * fromWeight + to.b * toWeight, from.a * fromWeight + to.a * toWeight};
		}

		[[nodiscard]] static constexpr ColorData getLerpVal(const float s, const std::span<const Color>& colors) {
			const size_t size = colors.size();
			const size_t bound = size - 1;
			const auto boundf = static_cast<float>(size);
			const Color& ca = colors[Math::clamp(static_cast<size_t>(s * boundf), 0ull, bound)];
			const Color& cb = colors[Math::clamp(static_cast<size_t>(s * boundf + 1), 0ull, bound)];

			const float n = s * boundf - static_cast<float>(static_cast<int>(s * boundf));
			const float i = 1.0f - n;
			return ColorData{ ca.r * i + cb.r * n, ca.g * i + cb.g * n, ca.b * i + cb.b * n, ca.a * i + cb.a * n};
		}

		static constexpr Color createLerp(const float s, const auto&... colors) {
			return Color{::Graphic::Color::getLerpVal(s, colors...)};
		}

		constexpr Color& lerp(const float s, const auto&... colors) {
			return this->set(::Graphic::Color::getLerpVal(s, colors...));
		}

		constexpr Color& lerp(const float s, const std::span<const Color>& colors) {
			return set(getLerpVal(s, colors));
		}
	};

	namespace Colors {
		constexpr Color WHITE{ 1, 1, 1, 1 };
		constexpr Color LIGHT_GRAY{ 0xbfbfbfff };
		constexpr Color GRAY{ 0x7f7f7fff };
		constexpr Color DARK_GRAY{ 0x3f3f3fff };
		constexpr Color BLACK{ 0, 0, 0, 1 };
		constexpr Color CLEAR{ 0, 0, 0, 0 };
		
		constexpr float WHITE_FLOAT_BITS = WHITE.toFloatBits();
		constexpr float CLEAR_FLOAT_BITS = CLEAR.toFloatBits();
		constexpr float BLACK_FLOAT_BITS = BLACK.toFloatBits();

		constexpr unsigned int WHITE_RGBA = WHITE.rgba();
		constexpr unsigned int CLEAR_RGBA = CLEAR.rgba();
		constexpr unsigned int BLACK_RGBA = BLACK.rgba();

		constexpr Color BLUE{ 0, 0, 1, 1 };
		constexpr Color NAVY{ 0, 0, 0.5f, 1 };
		constexpr Color ROYAL{ 0x4169e1ff };
		constexpr Color SLATE{ 0x708090ff };
		constexpr Color SKY{ 0x87ceebff };

		constexpr Color AQUA{ 0x85A2F3ff };

		constexpr Color AQUA_SKY = Color::createLerp(0.5f, AQUA, SKY);

		constexpr Color BLUE_SKY = Color::createLerp(0.745f, BLUE, SKY);

		constexpr Color CYAN{ 0, 1, 1, 1 };
		constexpr Color TEAL{ 0, 0.5f, 0.5f, 1 };

		constexpr Color GREEN{ 0x00ff00ff };
		constexpr Color ACID{ 0x7fff00ff };
		constexpr Color LIME{ 0x32cd32ff };
		constexpr Color FOREST{ 0x228b22ff };
		constexpr Color OLIVE{ 0x6b8e23ff };

		constexpr Color YELLOW{ 0xffff00ff };
		constexpr Color GOLD{ 0xffd700ff };
		constexpr Color GOLDENROD{ 0xdaa520ff };
		constexpr Color ORANGE{ 0xffa500ff };

		constexpr Color BROWN{ 0x8b4513ff };
		constexpr Color TAN{ 0xd2b48cff };
		constexpr Color BRICK{ 0xb22222ff };

		constexpr Color RED{ 0xff0000ff };
		constexpr Color RED_DUSK{ 0xDE6663ff };
		constexpr Color SCARLET{ 0xff341cff };
		constexpr Color CRIMSON{ 0xdc143cff };
		constexpr Color CORAL{ 0xff7f50ff };
		constexpr Color SALMON{ 0xfa8072ff };
		constexpr Color PINK{ 0xff69b4ff };
		constexpr Color MAGENTA{ 1, 0, 1, 1 };

		constexpr Color PURPLE{ 0xa020f0ff };
		constexpr Color VIOLET{ 0xee82eeff };
		constexpr Color MAROON{ 0xb03060ff };
	}
}

export
	template<>
	struct ::std::hash<Graphic::Color>{
		size_t operator()(const Graphic::Color& obj) const noexcept{
			return obj.hashCode();
		}
	};
