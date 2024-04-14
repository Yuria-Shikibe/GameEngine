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
import GL.Texture.TextureRegionRect;
import Geom.Vector2D;

export import UI.Align;

import Math;
import ext.Encoding;
import std;

export namespace Font {
	constexpr int MAX_CAHCE = 1000;

	using TextView = std::string_view;
	using TextString = std::string;

	std::unordered_map<TextView, const FontFlags*> parserableFonts{};
	std::unordered_map<TextView, Graphic::Color> parserableColors{};

	void registerParserableFont(const Font::FontFlags* const flag) {
		parserableFonts.insert_or_assign(flag->fullname(), flag);
	}

	void registerParserableFont(const TextView name, const Font::FontFlags* const flag) {
		parserableFonts.insert_or_assign(name, flag);
	}

	//TODO color managements
	void registerParserableColor(const TextView name, const Graphic::Color& color) {
		parserableColors.insert_or_assign(name, color);
	}

	struct GlyphDrawData {
		CharCode code{};
		int index{};

		const GL::TextureRegionRect* region{nullptr};
		Graphic::Color fontColor{};

		Geom::Vec2 src{};
		Geom::Vec2 end{};
		Geom::Point2U layoutPos{};

		Math::Range heightAlign{};

		[[nodiscard]] bool isEnd() const{
			return code == 0;
		}

		[[nodiscard]] bool isEndRow() const{
			return code == '\n';
		}

		[[nodiscard]] bool isBeginRow() const{
			return layoutPos.x == 0 && layoutPos.y != 0;
		}

		[[nodiscard]] bool isBegin() const{
			return layoutPos.isZero();
		}

		[[nodiscard]] Geom::Vec2 v00() const{return src;}
		[[nodiscard]] Geom::Vec2 v01() const{return {src.x, end.y};}
		[[nodiscard]] Geom::Vec2 v11() const{return end;}
		[[nodiscard]] Geom::Vec2 v10() const{return {end.x, src.y};}

		//{ src.x,  src.y - getBelowBaseHeight(),  getWidth(),  getFullHeight()};
		[[nodiscard]] Geom::Vec2 getBoundSrc() const{return {src.x,  src.y - getBelowBaseHeight()};}
		[[nodiscard]] Geom::Vec2 getBoundEnd() const{return {end.x,  src.y - getBelowBaseHeight() + getFullHeight()};}

		void move(const float x, const float y) {
			src.add(x, y);
			end.add(x, y);
		}

		void moveX(const float x) {
			src.x += x;
			end.x += x;
		}

		void moveY(const float y) {
			src.y += y;
			end.y += y;
		}

		[[nodiscard]] unsigned getRow() const{
			return layoutPos.y;
		}

		[[nodiscard]] unsigned getColumn() const{
			return layoutPos.x;
		}

		[[nodiscard]] float getAboveBaseHeight() const{
			return getFullHeight() - getBelowBaseHeight();
		}

		[[nodiscard]] float getBelowBaseHeight() const{
			return -heightAlign.from;
		}

		[[nodiscard]] float getWidth() const{
			return end.x - src.x;
		}

		[[nodiscard]] float getGlyphHeight() const{
			return end.y - src.y;
		}

		[[nodiscard]] float getFullHeight() const{
			return end.y - src.y + heightAlign.to;
		}

		[[nodiscard]] Geom::OrthoRectFloat getBound() const{
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

		void updateDrawbound(){
			drawBund = rawBound;
			drawBund.sclSize(scale, scale);
			drawBund.sclPos(scale, scale);
		}

		void setSCale(const float scale){
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
			return glyphs.empty();
		}

		[[nodiscard]] decltype(auto) takeValid() {
			return glyphs | std::ranges::views::all;
		}

		[[nodiscard]] decltype(auto) takeValid() const {
			return glyphs | std::ranges::views::all;
		}

		[[nodiscard]] const GlyphDrawData* find(const Geom::Point2U layoutPos) const;

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

		void setAlign(const Align::Mode align) {
			if(align & Align::Mode::top) {
				rawBound.setSrcY(-rawBound.getHeight());
			}else if(align & Align::Mode::bottom){
				rawBound.setSrcY(0.0f);
			}else { //centerY
				rawBound.setSrcY(-rawBound.getHeight() * 0.5f);
			}

			if(align & Align::Mode::right) {
				rawBound.setSrcX(-rawBound.getWidth());
			}else if(align & Align::Mode::left){
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

	ext::ObjectPool<GlyphLayout> layoutPool{250, [](GlyphLayout* item) {
		item->reset();
	}};

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
		Geom::Point2U currentLayoutPos{};

		Geom::Vec2 offset{};

		std::vector<GlyphDrawData*> currentLineData{};
		std::vector<std::function<void(const ModifierableData&)>> endlineOperation{};
		Geom::OrthoRectFloat currentLineBound{};

		//TODO uses stack for multi fallback?
		Graphic::Color currentColor = Graphic::Colors::WHITE;
		Graphic::Color fallbackColor = Graphic::Colors::WHITE;

		const FontFlags* defaultFont{nullptr};
		const FontFlags* currentFont{nullptr};
		const FontFlags* fallbackFont{nullptr};

		[[nodiscard]] explicit TypesettingContext(const FontFlags* font);

		[[nodiscard]] TypesettingContext() = default;

		void set(const FontFlags* const currentFont) {
			this->currentFont = currentFont;
			lineSpacing = currentFont->data->lineSpacingMin * 1.8f;
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

		const char TokenSignal = '$'; //$$for$
		//TODO switch this to '< >' to make this compatible with std::format
		const char TokenBeginCode = '<';
		const char TokenEndCode = '>';

		Font::FontAtlas* fontLib{nullptr};

		mutable TypesettingContext context{};

		std::unique_ptr<TokenParser> tokenParser{std::make_unique<TokenParser>()};
		std::unique_ptr<CharParser> charParser{std::make_unique<CharParser>()};

		/**
		 * \brief Customizable Fields
		 */
		[[nodiscard]] explicit GlyphParser(const FontFlags* const defFont) : context{defFont}{}

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

	void initParser(const FontFlags* defFont);
}
