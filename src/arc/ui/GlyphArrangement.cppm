//
// Created by Matrix on 2023/11/30.
//

module;

export module Font.GlyphArrangement;

import ext.Container.ObjectPool;
export import Font;

import Graphic.Color;
import ext.Container.ObjectPool;
import Geom.Rect_Orthogonal;
import GL.Texture.TextureRegion;
import Geom.Vector2D;

export import UI.Align;

import Math;
import ext.Encoding;
import std;

export namespace Font {
	constexpr int MAX_CAHCE = 1000;

	using TextView = std::string_view;
	using TextString = std::string;
	using TextLayoutPos = Geom::Point2US;


	std::unordered_map<TextView, const FontFace*> parserableFonts{};
	std::unordered_map<TextView, Graphic::Color> parserableColors{};

	void registerParserableFont(const Font::FontFace* const flag) {
		parserableFonts.insert_or_assign(flag->fullname(), flag);
	}

	void registerParserableFont(const TextView name, const Font::FontFace* const flag) {
		parserableFonts.insert_or_assign(name, flag);
	}

	//TODO color managements
	void registerParserableColor(const TextView name, const Graphic::Color& color) {
		parserableColors.insert_or_assign(name, color);
	}

	struct GlyphDrawData {
		CharCode code{};
		int index{};

		const GL::TextureRegion* region{nullptr};
		Graphic::Color fontColor{};

		Geom::Vec2 src{};
		Geom::Vec2 end{};
		TextLayoutPos layoutPos{};

		Math::Range heightAlign{};

		[[nodiscard]] bool isSentinel() const noexcept{
			return isEnd() || isBegin();
		}

		[[nodiscard]] constexpr bool isEnd() const noexcept{
			return code == 0;
		}

		[[nodiscard]] constexpr bool isEndRow() const noexcept{
			return code == '\n';
		}

		[[nodiscard]] constexpr bool isBeginRow() const noexcept{
			return layoutPos.x == 0 && layoutPos.y != 0;
		}

		[[nodiscard]] constexpr bool isBegin() const noexcept{
			return layoutPos.isZero();
		}

		[[nodiscard]] constexpr Geom::Vec2 v00() const noexcept{return src;}
		[[nodiscard]] constexpr Geom::Vec2 v01() const noexcept{return {src.x, end.y};}
		[[nodiscard]] constexpr Geom::Vec2 v11() const noexcept{return end;}
		[[nodiscard]] constexpr Geom::Vec2 v10() const noexcept{return {end.x, src.y};}

		//{ src.x,  src.y - getBelowBaseHeight(),  getWidth(),  getFullHeight()};
		[[nodiscard]] constexpr Geom::Vec2 getBoundSrc() const noexcept{return {src.x,  src.y - getBelowBaseHeight()};}
		[[nodiscard]] constexpr Geom::Vec2 getBoundEnd() const noexcept{return {end.x,  src.y - getBelowBaseHeight() + getFullHeight()};}

		constexpr void move(const float x, const float y) noexcept{
			src.add(x, y);
			end.add(x, y);
		}

		constexpr void moveX(const float x) noexcept{
			src.x += x;
			end.x += x;
		}

		constexpr void moveY(const float y) noexcept{
			src.y += y;
			end.y += y;
		}

		[[nodiscard]] constexpr unsigned getRow() const noexcept{
			return layoutPos.y;
		}

		[[nodiscard]] constexpr unsigned getColumn() const noexcept{
			return layoutPos.x;
		}

		[[nodiscard]] constexpr float getAboveBaseHeight() const noexcept{
			return getFullHeight() - getBelowBaseHeight();
		}

		[[nodiscard]] constexpr float getBelowBaseHeight() const noexcept{
			return -heightAlign.from;
		}

		[[nodiscard]] constexpr float getWidth() const noexcept{
			return end.x - src.x;
		}

		[[nodiscard]] constexpr float getGlyphHeight() const noexcept{
			return end.y - src.y;
		}

		[[nodiscard]] constexpr float getFullHeight() const noexcept{
			return end.y - src.y + heightAlign.to;
		}

		[[nodiscard]] constexpr Geom::OrthoRectFloat getBound() const noexcept{
			return Geom::OrthoRectFloat{ src.x,  src.y - getBelowBaseHeight(),  getWidth(),  getFullHeight()};
		}
	};

	class GlyphLayout { // NOLINT(*-pro-type-member-init)

		//TODO uses pools!
		std::vector<GlyphDrawData> glyphs{};
		Geom::OrthoRectFloat rawBound{};
		Geom::OrthoRectFloat drawBund{};
		float scale = 1.0f;


	public:
		Geom::Vec2 offset{};
		float maxWidth{std::numeric_limits<float>::max()};

		TextString lastText{};

		TextView getView(bool ignoreLastEnter = true) const{
			TextView view = lastText;
			if(!view.empty() && ignoreLastEnter){
				if(view.back() == '\n'){
					return view.substr(0, view.size() - 1);
				}
			}
			return view;
		}

		void updateDrawbound() noexcept{
			drawBund = rawBound;
			drawBund.sclSize(scale, scale);
			drawBund.sclPos(scale, scale);
		}

		void setSCale(const float scale) noexcept{
			this->scale = scale;
			updateDrawbound();
		}

		[[nodiscard]] float getScale() const{ return scale; }

		[[nodiscard]] Geom::OrthoRectFloat& getRawBound(){ return rawBound; }

		[[nodiscard]] const Geom::OrthoRectFloat& getDrawBound() const{ return drawBund; }

		[[nodiscard]] Geom::Vec2 getDrawOffset() const{
			return drawBund.getSrc() + offset;
		}

		[[nodiscard]] GlyphDrawData& front(){
			return glyphs.front();
		}

		[[nodiscard]] GlyphDrawData& back(){
			return glyphs.back();
		}

		[[nodiscard]] std::vector<GlyphDrawData>& getGlyphs(){
			return glyphs;
		}

		[[nodiscard]] bool empty() const{
			return glyphs.empty() || glyphs.front().code == 0;
		}

		[[nodiscard]] bool ignore() const{
			return glyphs.empty() || glyphs.front().code == 2;
		}

		[[nodiscard]] decltype(auto) takeValid() {
			return glyphs | std::ranges::views::all;
		}

		[[nodiscard]] decltype(auto) takeValid() const {
			return glyphs | std::ranges::views::all;
		}

		[[nodiscard]] const GlyphDrawData* find(const TextLayoutPos layoutPos) const;

		void reset() {
			lastText.clear();
			maxWidth = std::numeric_limits<float>::max();
			rawBound.set(0, 0, 0, 0);
			offset.setZero();
		}

		void clear() {
			rawBound.setSize(0, 0);
			rawBound.setSrc(0, 0);
			glyphs.clear();
		}

		void move(const float x, const float y) {
			offset.add(x, y);
		}

		void move(const Geom::Vec2& vec) {
			move(vec.x, vec.y);
		}

		void setAlign(const Align::Layout align) {
			if(align & Align::Layout::top) {
				rawBound.setSrcY(-rawBound.getHeight());
			}else if(align & Align::Layout::bottom){
				rawBound.setSrcY(0.0f);
			}else { //centerY
				rawBound.setSrcY(-rawBound.getHeight() * 0.5f);
			}

			if(align & Align::Layout::right) {
				rawBound.setSrcX(-rawBound.getWidth());
			}else if(align & Align::Layout::left){
				rawBound.setSrcX(0.0f);
			}else { //centerX
				rawBound.setSrcX(-rawBound.getWidth() * 0.5f);
			}

			updateDrawbound();
		}

		void render(const float alphaMask = 1.0f) const;

		void render(const float alphaMask, float progress) const;
	};

	//TODO cache support. maybe?
	// struct GlyphLayoutCache {
	// 	size_t frequency{0};
	//
	// 	std::shared_ptr<GlyphLayout> data{nullptr};
	// };
	//
	// std::unordered_map<std::string, GlyphLayoutCache> layoutCache(MAX_CAHCE);

	ext::ObjectPool<GlyphLayout> layoutPool{250};

	std::shared_ptr<GlyphLayout> obtainLayoutPtr() {
		return layoutPool.obtainShared();
	}

	struct ModifierableData;

	struct TypesettingContext {
		float lineSpacing{-1};
		float currentScale{1.0f};
		float paragraphSpacing{-1};

		float additionalYOffset{0.0f}; // This happens when the font changes, maybe

		/** x for column, y for row*/
		TextLayoutPos currentLayoutPos{};

		Geom::Vec2 offset{};

		std::vector<GlyphDrawData*> currentLineData{};
		std::vector<std::function<void(const ModifierableData&)>> endlineOperation{};
		Geom::OrthoRectFloat currentLineBound{};

		//TODO uses stack for multi fallback?
		Graphic::Color currentColor = Graphic::Colors::WHITE;
		Graphic::Color fallbackColor = Graphic::Colors::WHITE;

		const FontFace* defaultFont{nullptr};
		const FontFace* currentFont{nullptr};
		const FontFace* fallbackFont{nullptr};

		[[nodiscard]] explicit TypesettingContext(const FontFace* font);

		[[nodiscard]] TypesettingContext() = default;

		void set(const FontFace* const currentFont) {
			this->currentFont = currentFont;
			lineSpacing = currentFont->data->lineSpacingDef * 1.8f;
			paragraphSpacing = lineSpacing * 1.1f;
		}

		[[nodiscard]] float padTop_line() const {
			return currentScale * lineSpacing;
		}

		[[nodiscard]] float padTop_paragraph() const {
			return currentScale * paragraphSpacing;
		}

		void reset() {
			currentColor = fallbackColor = Graphic::Colors::WHITE;
			currentScale = 1.0f;
			set(defaultFont);
			fallbackFont = defaultFont;

			offset.setZero();
			additionalYOffset = 0;
			currentLayoutPos.setZero();
			currentLineBound = {};
		}
	};

	struct ModifierableData {
		TypesettingContext& context;
		Geom::Vec2& cursorPos;
		const Font::CharData*& charData;
		GlyphLayout& layout;

		[[nodiscard]] ModifierableData(TypesettingContext& context, Geom::Vec2& vec2,
			const Font::CharData*& data, GlyphLayout& layout)
			: context(context),
			cursorPos(vec2),
			charData(data),
			layout(layout) {
		}
	};

namespace ParserFunctions {
	void setScl(const ModifierableData& data, float target);

	void resetScl(const ModifierableData& data) {
		setScl(data, 1.0f);
	}

	void endLine(const ModifierableData& data);

	void pushData(const CharCode code, const int index, const CharData* charData, const ModifierableData& data);
}

	class TokenParser {
	public:
		using ModifierFunc = std::function<void(unsigned, TextView, const ModifierableData& data)>;
		virtual ~TokenParser() = default;

		bool reserveTokenSentinal = false;
		std::unordered_map<TextView, ModifierFunc> modifier{};
		ModifierFunc fallBackModifier{nullptr};

		explicit operator bool() const{
			return static_cast<bool>(fallBackModifier) || !modifier.empty();
		}

		virtual void parse(unsigned curIndex, TextView token, const ModifierableData& data) const;
	};

	class CharParser{
	public:
		virtual ~CharParser() = default;

		/** @return true if this char should be passed*/
		//TODO wchar_t support
		std::unordered_map<CharCode, std::function<void(const ModifierableData& data)>> modifier{};
		std::unordered_set<CharCode> shouldNotContinueSet{};

		virtual void parse(const CharCode token, const ModifierableData& data) const {
			modifier.at(token)(data);
		}

		[[nodiscard]] bool shouldNotContinue(const CharCode code) const{
			return shouldNotContinueSet.contains(code);
		}

		[[nodiscard]] bool contains(const CharCode c) const {
			return modifier.contains(c);
		}

		void registerDefParser(){
			// modifier[' '] = [](const ModifierableData& data) {
			// 	data.cursorPos.add(data.context.spaceSpaceing * data.context.currentScale, 0);
			// };

			modifier['\n'] = [](const ModifierableData& data) {
				ParserFunctions::endLine(data);
			};

			modifier['\r'] = [](const ModifierableData& data) {};

			shouldNotContinueSet.insert('\n');
		}
	};

	class GlyphParser {
	public:
		virtual ~GlyphParser() = default;

		static constexpr char TokenSignal = '$'; //$$for$
		//TODO switch this to '< >' to make this compatible with std::format
		static constexpr char TokenBeginCode = '<';
		static constexpr char TokenEndCode = '>';

		Font::FontStorage* loadedFonts{nullptr};

		mutable TypesettingContext context{};

		std::unique_ptr<TokenParser> tokenParser{std::make_unique<TokenParser>()};
		std::unique_ptr<CharParser> charParser{std::make_unique<CharParser>()};

		/**
		 * \brief Customizable Fields
		 */
		[[nodiscard]] explicit GlyphParser(const FontFace* const defFont) : context{defFont}{}

		virtual void parse(const std::shared_ptr<GlyphLayout>& layout) const;

		void parseWith(const std::shared_ptr<GlyphLayout>& layout, TextString&& str, const float maxWidth = std::numeric_limits<float>::max()) const{
			bool requiresRelayout = layout->maxWidth != maxWidth;

			if(requiresRelayout){
				layout->maxWidth = maxWidth;
			}

			if(layout->lastText != str){
				requiresRelayout = true;
				layout->lastText = std::move(str);
			}

			if(requiresRelayout)parse(layout);
		}

		void parseWith(const std::shared_ptr<GlyphLayout>& layout, const TextView str, const float maxWidth = std::numeric_limits<float>::max()) const{
			bool requiresRelayout = layout->maxWidth != maxWidth;

			if(layout->lastText != str){
				requiresRelayout = true;
				layout->maxWidth = maxWidth;
				layout->lastText = str;
			}

			if(requiresRelayout)parse(layout);
		}

		void parseWith(const std::shared_ptr<GlyphLayout>& layout, const TextView::value_type* str, const float maxWidth = std::numeric_limits<float>::max()) const{
			parseWith(layout, TextView{str}, maxWidth);
		}

		void parseWith(const std::shared_ptr<GlyphLayout>& layout, const float maxWidth = std::numeric_limits<float>::max(), const bool forceLayout = false) const{
			if(layout->maxWidth != maxWidth || forceLayout){

				layout->maxWidth = maxWidth;
				parse(layout);
			}
		}

		[[nodiscard]] std::shared_ptr<GlyphLayout> parse(const TextView text) const {
			const auto layout = obtainLayoutPtr();

			layout->lastText = text;

			parse(layout);

			return layout;
		}

		std::shared_ptr<GlyphLayout> operator()(const TextView text) const {
			//TODO complete glyph layout cache;
			/*if(
				const auto itr = layoutCache.find(text);
				itr != layoutCache.end()
			) {
				itr->second.frequency++;
				return itr->second.data;
			}*/

			return parse(text);
		}

		[[nodiscard]] GlyphParser() : GlyphParser(nullptr){

		}
	};

	std::unique_ptr<GlyphParser> defGlyphParser = nullptr;
	std::unique_ptr<GlyphParser> forwardParser = nullptr;

	void initParser(const FontFace* defFont);
}
