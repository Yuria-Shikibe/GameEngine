module ;

export module Graphic.Color;

import <string>;
import Math;
import Math.Bit;
import Geom.Vector3D;
import <sstream>;

export namespace Graphic{
	using std::max;
	using std::min;

	class Color{
	public:
		float r = 0, g = 0, b = 0, a = 0;

		using colorBits = unsigned int;

		Color() = default;

		explicit Color(const colorBits rgba8888V){
			rgba8888(rgba8888V);
		}

		Color(const float tr, const float tg, const float tb, const float ta){
			r = tr;
			g = tg;
			b = tb;
			a = ta;
			clamp();
		}

		Color(const float r, const float g, const float b): Color(r, g, b, 1){}

		Color(Color&& color) = default;

		Color(const Color& other) = default;

		Color& operator=(const Color& other){
			if (this == &other) return *this;
			return set(other);  // NOLINT(misc-unconventional-assign-operator)
		}

		Color& operator=(Color&& other) noexcept{
			if (this == &other) return *this;
			return set(other);  // NOLINT(misc-unconventional-assign-operator)
		}

		static std::size_t hash_value(const Color& obj){
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
			return os << obj.toString();
		}

		friend bool operator==(const Color& lhs, const Color& rhs){
			return lhs.equals(rhs);
		}

		friend bool operator!=(const Color& lhs, const Color& rhs){
			return !lhs.equals(rhs);
		}

		static Color& valueOf(const std::string& hex){
			Color c{};
			return valueOf(c, hex);
		}

		static Color& valueOf(Color& color, const std::string& hex){
			const int offset = hex[0] == '#' ? 1 : 0;

			const int r = parseHex(hex, offset, offset + 2);
			const int g = parseHex(hex, offset + 2, offset + 4);
			const int b = parseHex(hex, offset + 4, offset + 6);
			const int a = hex.length() - offset != 8 ? 255 : parseHex(hex, offset + 6, offset + 8);
			return color.set(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f);
		}

		static int parseHex(const std::string& string, const int from, const int to){
			int total = 0;
			for (int i = from; i < to; i++) {
				total += Math::charToDigitValue(string[i], 16) * (i == from ? 16 : 1);
			}
			return total;
		}

		static float toFloatBits(const int r, const int g, const int b, const int a){
			const int color = a << 24 | b << 16 | g << 8 | r;
			return intToFloatColor(color);
		}

		static float toFloatBits(const float r, const float g, const float b, const float a){
			const int color = static_cast<int>(255 * a) << 24 | static_cast<int>(255 * b) << 16 | static_cast<int>(255 * g) << 8 | static_cast<int>(255 * r);
			return intToFloatColor(color);
		}

		static double toDoubleBits(const float r, const float g, const float b, const float a){
			return Math::longBitsToDouble(rgba8888(r, g, b, a) & Math::INT_MASK_BACK);
		}

		static double toDoubleBits(const int r, const int g, const int b, const int a){
			return toDoubleBits(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f);
		}

		Color& fromDouble(const double value){
			return rgba8888(static_cast<int>(Math::doubleToUnsignedLongBits(value)));
		}

		static colorBits abgr(const int tr, const int tg, const int tb, const int ta){
			return ta << 24 | tb << 16 | tg << 8 | tr;
		}

		static colorBits alpha(const float alpha){
			return static_cast<colorBits>(alpha * 255.0f);
		}

		static colorBits luminanceAlpha(const float luminance, const float alpha){
			return static_cast<colorBits>(luminance * 255.0f) << 8 | static_cast<colorBits>(alpha * 255.0f);
		}

		static colorBits rgb565(const float r, const float g, const float b){
			return static_cast<colorBits>(r * 31) << 11 | static_cast<colorBits>(g * 63) << 5 | static_cast<colorBits>(b * 31);
		}

		static colorBits rgba4444(const float r, const float g, const float b, const float a){
			return static_cast<colorBits>(r * 15) << 12 | static_cast<colorBits>(g * 15) << 8 | static_cast<colorBits>(b * 15) << 4 | static_cast<colorBits>(a * 15);
		}

		static colorBits rgb888(const float r, const float g, const float b){
			return static_cast<colorBits>(r * 255) << 16 | static_cast<colorBits>(g * 255) << 8 | static_cast<colorBits>(b * 255);
		}

		static colorBits rgba8888(const float r, const float g, const float b, const float a){
			return static_cast<colorBits>(r * 255) << 24 | static_cast<colorBits>(g * 255) << 16 | static_cast<colorBits>(b * 255) << 8 | static_cast<colorBits>(a * 255);
		}

		static colorBits argb8888(const float a, const float r, const float g, const float b){
			return static_cast<colorBits>(a * 255) << 24 | static_cast<colorBits>(r * 255) << 16 | static_cast<colorBits>(g * 255) << 8 | static_cast<colorBits>(b * 255);
		}

		static int packRgba(const int r, const int g, const int b, const int a){
			return r << 24 | g << 16 | b << 8 | a;
		}

		[[nodiscard]] colorBits rgb565() const{
			return static_cast<colorBits>(r * 31) << 11 | static_cast<colorBits>(g * 63) << 5 | static_cast<colorBits>(b * 31);
		}

		[[nodiscard]] colorBits rgba4444() const{
			return static_cast<colorBits>(r * 15) << 12 | static_cast<colorBits>(g * 15) << 8 | static_cast<colorBits>(b * 15) << 4 | static_cast<colorBits>(a * 15);
		}

		[[nodiscard]] colorBits rgb888() const{
			return static_cast<colorBits>(r * 255) << 16 | static_cast<colorBits>(g * 255) << 8 | static_cast<colorBits>(b * 255);
		}

		[[nodiscard]] colorBits rgba8888() const{
			return static_cast<colorBits>(r * 255) << 24 | static_cast<colorBits>(g * 255) << 16 | static_cast<colorBits>(b * 255) << 8 | static_cast<colorBits>(a * 255);
		}

		[[nodiscard]] colorBits argb8888() const{
			return static_cast<colorBits>(a * 255) << 24 | static_cast<colorBits>(r * 255) << 16 | static_cast<colorBits>(g * 255) << 8 | static_cast<colorBits>(b * 255);
		}

		Color& rgb565(const colorBits value){
			r = static_cast<float>((value & 0x0000F800) >> 11) / 31.0f;
			g = static_cast<float>((value & 0x000007E0) >> 5) / 63.0f;
			b = static_cast<float>((value & 0x0000001F)) / 31.0f;
			return *this;
		}

		Color& rgba4444(const colorBits value){
			r = static_cast<float>((value & 0x0000f000) >> 12) / 15.0f;
			g = static_cast<float>((value & 0x00000f00) >> 8) / 15.0f;
			b = static_cast<float>((value & 0x000000f0) >> 4) / 15.0f;
			a = static_cast<float>((value & 0x0000000f)) / 15.0f;
			return *this;
		}

		Color& rgb888(const colorBits value){
			r = static_cast<float>((value & 0x00ff0000) >> 16) / 255.0f;
			g = static_cast<float>((value & 0x0000ff00) >> 8) / 255.0f;
			b = static_cast<float>((value & 0x000000ff)) / 255.0f;
			return *this;
		}

		Color& rgba8888(const colorBits value){
			r = static_cast<float>((value & 0xff000000) >> 24) / 255.0f;
			g = static_cast<float>((value & 0x00ff0000) >> 16) / 255.0f;
			b = static_cast<float>((value & 0x0000ff00) >> 8) / 255.0f;
			a = static_cast<float>((value & 0x000000ff)) / 255.0f;
			return *this;
		}

		Color& argb8888(const colorBits value){
			a = static_cast<float>((value & 0xff000000) >> 24) / 255.0f;
			r = static_cast<float>((value & 0x00ff0000) >> 16) / 255.0f;
			g = static_cast<float>((value & 0x0000ff00) >> 8) / 255.0f;
			b = static_cast<float>((value & 0x000000ff)) / 255.0f;
			return *this;
		}

		Color& abgr8888(const float value){
			const unsigned int c = floatToIntColor(value);
			a = static_cast<float>((c & 0xff000000) >> 24) / 255.0f;
			b = static_cast<float>((c & 0x00ff0000) >> 16) / 255.0f;
			g = static_cast<float>((c & 0x0000ff00) >> 8) / 255.0f;
			r = static_cast<float>((c & 0x000000ff)) / 255.0f;
			return *this;
		}

		static Color grays(const float value){
			return Color{ value, value, value };
		}

		static Color rgb(const int r, const int g, const int b){
			return Color{ (static_cast<float>(r) / 255.0f), (static_cast<float>(g) / 255.0f), (static_cast<float>(b) / 255.0f) };
		}

		static unsigned floatToIntColor(const float value){
			colorBits intBits = Math::floatToUnsignedIntBits(value);
			intBits |= static_cast<colorBits>(static_cast<float>(intBits >> 24) * (255.0f / 254.0f)) << 24;
			return intBits;
		}

		static float intToFloatColor(const colorBits value){
			return Math::intBitsToFloat(value & 0xfeffffff);
		}

		[[nodiscard]] float diff(const Color& other) const {
			return abs(hue() - other.hue()) / 360 + abs(value() - other.value()) + abs(saturation() - other.saturation());
		}

		[[nodiscard]] colorBits rgba() const{
			return rgba8888();
		}

		Color& set(const Color& color){
			this->r = color.r;
			this->g = color.g;
			this->b = color.b;
			this->a = color.a;
			return *this;
		}

		Color& set(const Geom::Vector3D vec){
			return set(vec.x, vec.y, vec.z);
		}

		Color& mul(const Color& color){
			this->r *= color.r;
			this->g *= color.g;
			this->b *= color.b;
			this->a *= color.a;
			return clamp();
		}

		Color& mul_rgb(const float value){
			this->r *= value;
			this->g *= value;
			this->b *= value;
			return clamp();
		}

		Color& mul_rgba(const float value){
			this->r *= value;
			this->g *= value;
			this->b *= value;
			this->a *= value;
			return clamp();
		}

		Color& add(const Color& color){
			this->r += color.r;
			this->g += color.g;
			this->b += color.b;
			return clamp();
		}

		Color& sub(const Color& color){
			this->r -= color.r;
			this->g -= color.g;
			this->b -= color.b;
			return clamp();
		}

		Color& clamp(){
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

		Color& setForce(const float tr, const float tg, const float tb, const float ta){
			this->r = tr;
			this->g = tg;
			this->b = tb;
			this->a = ta;

			return *this;
		}

		Color& set(const float tr, const float tg, const float tb, const float ta){
			this->r = tr;
			this->g = tg;
			this->b = tb;
			this->a = ta;
			return clamp();
		}

		Color& set(const float tr, const float tg, const float tb){
			this->r = tr;
			this->g = tg;
			this->b = tb;
			return clamp();
		}

		Color& set(const int rgba){
			return rgba8888(rgba);
		}

		[[nodiscard]] float sum() const{
			return r + g + b;
		}

		Color& add(const float tr, const float tg, const float tb, const float ta){
			this->r += tr;
			this->g += tg;
			this->b += tb;
			this->a += ta;
			return clamp();
		}

		Color& add(const float tr, const float tg, const float tb){
			this->r += tr;
			this->g += tg;
			this->b += tb;
			return clamp();
		}

		Color& sub(const float tr, const float tg, const float tb, const float ta){
			this->r -= tr;
			this->g -= tg;
			this->b -= tb;
			this->a -= ta;
			return clamp();
		}

		Color& sub(const float tr, const float tg, const float tb){
			this->r -= tr;
			this->g -= tg;
			this->b -= tb;
			return clamp();
		}

		Color& inv(){
			r = 1 - r;
			g = 1 - g;
			b = 1 - b;
			return *this;
		}

		Color& setR(const float tr){
			this->r -= tr;
			return *this;
		}

		Color& setG(const float tg){
			this->g -= tg;
			return *this;
		}

		Color& setB(const float tb){
			this->b -= tb;
			return *this;
		}

		Color& setA(const float ta){
			this->a = ta;
			return *this;
		}

		Color& mulA(const float ta){
			this->a *= ta;
			return *this;
		}

		Color& mul(const float tr, const float tg, const float tb, const float ta){
			this->r *= tr;
			this->g *= tg;
			this->b *= tb;
			this->a *= ta;
			return clamp();
		}

		Color& lerp(const Color& target, const float t){
			this->r += t * (target.r - this->r);
			this->g += t * (target.g - this->g);
			this->b += t * (target.b - this->b);
			this->a += t * (target.a - this->a);
			return clamp();
		}

		Color& lerp(const float tr, const float tg, const float tb, const float ta, const float t){
			this->r += t * (tr - this->r);
			this->g += t * (tg - this->g);
			this->b += t * (tb - this->b);
			this->a += t * (ta - this->a);
			return clamp();
		}

		Color& preMultiplyAlpha(){
			r *= a;
			g *= a;
			b *= a;
			return *this;
		}

		Color& write(Color& to) const{
			return to.set(*this);
		}

		static float TMP_HSV[3];

		[[nodiscard]] float hue() const{
			toHsv(TMP_HSV);
			return TMP_HSV[0];
		}

		[[nodiscard]] float saturation() const{
			toHsv(TMP_HSV);
			return TMP_HSV[1];
		}

		[[nodiscard]] float value() const{
			toHsv(TMP_HSV);
			return TMP_HSV[2];
		}

		Color& hue(const float amount){
			toHsv(TMP_HSV);
			TMP_HSV[0] = amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& saturation(const float amount){
			toHsv(TMP_HSV);
			TMP_HSV[1] = amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& value(const float amount){
			toHsv(TMP_HSV);
			TMP_HSV[2] = amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& shiftHue(const float amount){
			toHsv(TMP_HSV);
			TMP_HSV[0] += amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& shiftSaturation(const float amount){
			toHsv(TMP_HSV);
			TMP_HSV[1] += amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		Color& shiftValue(const float amount){
			toHsv(TMP_HSV);
			TMP_HSV[2] += amount;
			fromHsv(TMP_HSV);
			return *this;
		}

		[[nodiscard]] bool equals(const Color& o) const{
			if (this == &o) return true;
			return abgr() == o.abgr();
		}

		[[nodiscard]] size_t hashCode() const{
			return rgba8888();
		}

		[[nodiscard]] float toFloatBits() const{
			const colorBits color = static_cast<colorBits>(255 * a) << 24 | static_cast<colorBits>(255 * b) << 16 | static_cast<colorBits>(255 * g) << 8 | static_cast<colorBits>(255 * r);
			return intToFloatColor(color);
		}

		[[nodiscard]] double toDoubleBits() const{
			return toDoubleBits(r, g, b, a);
		}

		[[nodiscard]] colorBits abgr() const{
			return static_cast<colorBits>(255 * a) << 24 | static_cast<colorBits>(255 * b) << 16 | static_cast<colorBits>(255 * g) << 8 | static_cast<colorBits>(255 * r);
		}

		[[nodiscard]] std::string toString() const{
			std::stringstream ss;
			ss << std::hex << (static_cast<colorBits>(255 * r) << 24 | static_cast<colorBits>(255 * g) << 16 | static_cast<colorBits>(255 * b) << 8 | static_cast<colorBits>(255 * a));


			if (const int paddingSize = 8 - static_cast<int>(ss.str().size()); paddingSize > 0) {
				const std::string hex = ss.str();
				ss.str(std::string(paddingSize, '0'));
				ss << hex;
			}

			return ss.str();
		}

		Color& fromHsv(const float h, const float s, const float v){
			const float x = fmod(h / 60.0f + 6, static_cast<float>(6));
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

		Color& fromHsv(float hsv[]){
			return fromHsv(hsv[0], hsv[1], hsv[2]);
		}

		Color HSVtoRGB(const float h, const float s, const float v, const float alpha){
			Color c = HSVtoRGB(h, s, v);
			c.a = alpha;
			return c;
		}

		Color HSVtoRGB(const float h, const float s, const float v){
			Color c{ 1, 1, 1, 1 };
			HSVtoRGB(h, s, v, c);
			return c;
		}

		float* toHsv(float hsv[]) const{
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

		Color& HSVtoRGB(float h, float s, float v, Color& targetColor){
			if (h == 360) h = 359;
			h = static_cast<float>(max(0.0f, min(360.0f, h)));
			s = static_cast<float>(max(0.0f, min(100.0f, s)));
			v = static_cast<float>(max(0.0f, min(100.0f, v)));
			s /= 100.0f;
			v /= 100.0f;
			h /= 60.0f;
			const int i = floor(h);
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

		static int clampf(const float value){
			return min(max(static_cast<int>(value), 0), 255);
		}

		static colorBits ri(const colorBits rgba){
			return (rgba & 0xff000000) >> 24;
		}

		static colorBits gi(const colorBits rgba){
			return (rgba & 0x00ff0000) >> 16;
		}

		static colorBits bi(const colorBits rgba){
			return (rgba & 0x0000ff00) >> 8;
		}

		static colorBits ai(const colorBits rgba){
			return rgba & 0x000000ff;
		}

		static colorBits muli(const colorBits ca, const colorBits cb){
			const colorBits
					rV = (ca & 0xff000000) >> 24,
					gV = (ca & 0x00ff0000) >> 16,
					bV = (ca & 0x0000ff00) >> 8,
					aV = ca & 0x000000ff,
					r2 = (cb & 0xff000000) >> 24,
					g2 = (cb & 0x00ff0000) >> 16,
					b2 = (cb & 0x0000ff00) >> 8,
					a2 = cb & 0x000000ff;
			return clampf(static_cast<float>(rV * r2) / 255.0f) << 24 | clampf(static_cast<float>(gV * g2) / 255.0f) << 16 | clampf(static_cast<float>(bV * b2) / 255.0f) << 8 | clampf(static_cast<float>(aV * a2) / 255.0f);
		}

		static colorBits muli(const colorBits rgba, const float value){
			const colorBits
					rV = (rgba & 0xff000000) >> 24,
					gV = (rgba & 0x00ff0000) >> 16,
					bV = (rgba & 0x0000ff00) >> 8,
					aV = rgba & 0x000000ff;
			return clampf(static_cast<float>(rV) * value) << 24 | clampf(static_cast<float>(gV) * value) << 16 | clampf(static_cast<float>(bV) * value) << 8 | aV;
		}


		template <int size>
		static Color createLerp(const Color(&arr)[size], const float s) {
			const Color ca = arr[Math::clamp(static_cast<int>(s * (size - 1)), 0, size - 1)];
			const Color cb = arr[Math::clamp(static_cast<int>(s * (size - 1) + 1), 0, size - 1)];

			const float n = s * (size - 1) - static_cast<int>(s * (size - 1));
			const float i = 1.0f - n;
			return { ca.r * i + cb.r * n, ca.g * i + cb.g * n, ca.b * i + cb.b * n, ca.a * i + cb.a * n };
		}

		template <int size>
		Color& lerp(const Color(&arr)[size], const float s) {
			const Color ca = arr[Math::clamp(static_cast<int>(s * (size - 1)), 0, size - 1)];
			const Color cb = arr[Math::clamp(static_cast<int>(s * (size - 1) + 1), 0, size - 1)];

			const float n = s * (size - 1) - static_cast<int>(s * (size - 1));
			const float i = 1.0f - n;
			return set(ca.r * i + cb.r * n, ca.g * i + cb.g * n, ca.b * i + cb.b * n, ca.a * i + cb.a * n);
		}
	};

	namespace Colors {
		const Color WHITE{ 1, 1, 1, 1 };
		const Color LIGHT_GRAY{ 0xbfbfbfff };
		const Color GRAY{ 0x7f7f7fff };
		const Color DARK_GRAY{ 0x3f3f3fff };
		const Color BLACK{ 0, 0, 0, 1 };
		const Color CLEAR{ 0, 0, 0, 0 };
		
		const float WHITE_FLOAT_BITS = WHITE.toFloatBits();
		const float CLEAR_FLOAT_BITS = CLEAR.toFloatBits();
		const float BLACK_FLOAT_BITS = BLACK.toFloatBits();

		const unsigned int WHITE_RGBA = WHITE.rgba();
		const unsigned int CLEAR_RGBA = CLEAR.rgba();
		const unsigned int BLACK_RGBA = BLACK.rgba();

		const Color BLUE{ 0, 0, 1, 1 };
		const Color NAVY{ 0, 0, 0.5f, 1 };
		const Color ROYAL{ 0x4169e1ff };
		const Color SLATE{ 0x708090ff };
		const Color SKY{ 0x87ceebff };
		const Color CYAN{ 0, 1, 1, 1 };
		const Color TEAL{ 0, 0.5f, 0.5f, 1 };

		const Color GREEN{ 0x00ff00ff };
		const Color ACID{ 0x7fff00ff };
		const Color LIME{ 0x32cd32ff };
		const Color FOREST{ 0x228b22ff };
		const Color OLIVE{ 0x6b8e23ff };

		const Color YELLOW{ 0xffff00ff };
		const Color GOLD{ 0xffd700ff };
		const Color GOLDENROD{ 0xdaa520ff };
		const Color ORANGE{ 0xffa500ff };

		const Color BROWN{ 0x8b4513ff };
		const Color TAN{ 0xd2b48cff };
		const Color BRICK{ 0xb22222ff };

		const Color RED{ 0xff0000ff };
		const Color SCARLET{ 0xff341cff };
		const Color CRIMSON{ 0xdc143cff };
		const Color CORAL{ 0xff7f50ff };
		const Color SALMON{ 0xfa8072ff };
		const Color PINK{ 0xff69b4ff };
		const Color MAGENTA{ 1, 0, 1, 1 };

		const Color PURPLE{ 0xa020f0ff };
		const Color VIOLET{ 0xee82eeff };
		const Color MAROON{ 0xb03060ff };
	}
}