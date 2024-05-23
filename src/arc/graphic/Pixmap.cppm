module;

export module Graphic.Pixmap;

import ext.RuntimeException;
import GL.Texture.Texture2D;
import GL.Buffer.FrameBuffer;
import OS.File;
import Image;
import ext.Concepts;
import Graphic.Color;
import Math;
import std;

using Color = Graphic::Color;
using ColorBits = Graphic::Color::ColorBits;

export namespace Graphic{
    class Pixmap{
    public:
        using DataType = unsigned char;
    protected:
        int width{ 1 };
        int height{ 1 };
        //TODO is this necessay?
        //unsigned int bpp{ 4 };

        std::unique_ptr<DataType[]> bitmapData{nullptr};

    public:
        static constexpr int Channels = 4; //FOR RGBA

        [[nodiscard]] Pixmap() = default;

        [[nodiscard]] int getWidth() const {
            return width;
        }

        [[nodiscard]] int getHeight() const {
            return height;
        }

        // [[nodiscard]] unsigned getBpp() const {
        //     return bpp;
        // }

        [[nodiscard]] DataType* release() && {
            return bitmapData.release();
        }

        [[nodiscard]] DataType* data() const & {
            return bitmapData.get();
        }

        [[nodiscard]] std::unique_ptr<DataType[]> data() && {
            return std::move(bitmapData);
        }

        static Pixmap createEmpty(const int width, const int height){
            Pixmap map{};
            map.width = width;
            map.height = height;
            return map;
        }

        [[nodiscard]] Pixmap(const int width, const int height)
            : width(width),
              height(height) {

            create(width, height);
        }

        explicit Pixmap(const OS::File& file){
            loadFrom(file);
        }

        explicit Pixmap(const GL::Texture2D& texture2D)
            : width(texture2D.getWidth()),
              height(texture2D.getHeight()) {
            loadFrom(texture2D);
        }

        Pixmap(const int width, const int height, std::unique_ptr<DataType[]>&& data)
                    : width(width),
                      height(height) {
            this->bitmapData = std::move(data);
        }

        Pixmap(const Pixmap& other)
            : width(other.width),
              height(other.height),
              //bpp(other.bpp),
              bitmapData(other.copyData()) {

        }

        Pixmap(Pixmap&& other) noexcept
            : width(other.width),
              height(other.height),
              //bpp(other.bpp),
              bitmapData(std::move(other.bitmapData)) {
        }

        Pixmap& operator=(const Pixmap& other) {
            if(this == &other) return *this;
            width = other.width;
            height = other.height;
            //bpp = other.bpp;
            bitmapData = other.copyData();
            return *this;
        }

        Pixmap& operator=(Pixmap&& other) noexcept {
            if(this == &other) return *this;
            width = other.width;
            height = other.height;
            //bpp = other.bpp;
            bitmapData = std::move(other.bitmapData);
            return *this;
        }

        [[nodiscard]] bool valid() const {
            return bitmapData != nullptr && pixelSize() > 0;
        }

        void create(const int width, const int height){
            bitmapData.reset();
            this->width = width;
            this->height = height;

            auto size = this->size();
            if(!size)return;

            bitmapData = std::make_unique<DataType[]>(size);
        }

        void free() {
            width = height = 0;
            if(bitmapData)bitmapData.reset(nullptr);
        }

        void fill(const Color color) const{
            std::fill_n(reinterpret_cast<ColorBits*>(bitmapData.get()), pixelSize(), color.rgba());
        }

        DataType& operator [](const size_t index){
            return bitmapData[index];
        }

        void mix(const Color color, const float t) const{
            for(int i = 0; i < pixelSize(); ++i){
                Color src = get(i);
                set(i, src.lerpRGB(color, t));
            }
        }

        void mulWhite() const{
            for(int i = 0; i < pixelSize(); ++i){
                setRaw(i, getRaw(i) | 0x00'ff'ff'ff);
            }
        }

        void loadFrom(const OS::File& file) {
            int b;
            this->bitmapData = ext::loadPng(file, width, height, b, Channels);
        }

        void loadFrom(const GL::Texture2D& texture2D) {
            bitmapData = texture2D.copyData();
            width = texture2D.getWidth();
            height = texture2D.getHeight();
        }

        void write(const OS::File& file, const bool autoCreate = false) const {
            if(!file.exist()) {
                bool result = false;

                if(autoCreate)result = file.createFile();

                if(!file.exist() || !result)throw ext::RuntimeException{"Inexist File!"};
            }

            // ReSharper disable once CppTooWideScopeInitStatement
            std::string&& ext = file.extension();

            if(ext == ".png") {
                ext::writePng(file, width, height, Channels, bitmapData.get());
            }else if(ext == ".bmp") {
                ext::writeBmp(file, width, height, Channels, bitmapData.get());
            }
        }

        void clear() const{
            std::fill_n(this->bitmapData.get(), size(), 0);
        }

        //TODO fix these two function!
        [[nodiscard]] GL::Texture2D genTex() const {
            return GL::Texture2D{width, height, copyData()};
        }

        [[nodiscard]] GL::Texture2D genTex_move() && {
            GL::Texture2D tex{width, height, std::move(bitmapData)};
            width = height = 0;
            return tex;
        }

        [[nodiscard]] constexpr auto dataIndex(const int x, const int y) const {
            if(x > width || y > height)throw ext::RuntimeException{"Array Index Out Of Bound!"};
            return (y * width + x);
        }

        [[nodiscard]] std::unique_ptr<DataType[]> copyData() const {

            const auto size = this->size();
            auto ptr = std::make_unique<DataType[]>(size);
            std::memcpy(ptr.get(), bitmapData.get(), size);

            return std::move(ptr);
        }

        void copyFrom(const DataType* data, const bool flipY = false) const {
            if(flipY){
                const size_t rowDataCount = static_cast<size_t>(getWidth()) * Channels;

                for(int y = 0; y < height; ++y){
                    std::memcpy(bitmapData.get() + rowDataCount * y, data + (getHeight() - y - 1) * rowDataCount, rowDataCount);
                }
            }else{
                std::memcpy(bitmapData.get(), data, size());
            }
        }

        [[nodiscard]] constexpr auto pixelSize() const {
            return width * height;
        }

        [[nodiscard]] constexpr auto size() const {
            return width * height * Channels;
        }

        void setRaw(const int index, const ColorBits colorBit) const{
            auto* ptr = reinterpret_cast<ColorBits*>(bitmapData.get() + index * Channels) ;
            *ptr = colorBit;
        }

        void setRaw(const int x, const int y, const ColorBits colorBit) const{
            setRaw(dataIndex(x, y), colorBit);
        }

        void set(const int index, const Graphic::Color& color) const{
            setRaw(index, color.rgba8888());
        }

        void set(const int x, const int y, const Graphic::Color& color) const{
            setRaw(x, y, color.rgba8888());
        }

        [[nodiscard]] ColorBits getRaw(const int index) const {
            const auto* ptr = reinterpret_cast<ColorBits*>(bitmapData.get() + index * Channels);
            return *ptr;
        }

        [[nodiscard]] ColorBits getRaw(const int x, const int y) const {
            return getRaw(dataIndex(x, y));
        }

        [[nodiscard]] Graphic::Color get(const int index) const {
            Color color{};
            return color.rgba8888(getRaw(index));
        }

        [[nodiscard]] Graphic::Color get(const int x, const int y) const {
            return get(dataIndex(x, y));
        }

        void each(Concepts::Invokable<void(Pixmap&, int, int)> auto&& func) {
            for(int x = 0; x < width; x++) {
                for(int y = 0; y < height; ++y) {
                    func(*this, x, y);
                }
            }
        }

        void each(Concepts::Invokable<void(const Pixmap&, int, int)> auto&& func) const {
            for(int x = 0; x < width; x++) {
                for(int y = 0; y < height; ++y) {
                    func(*this, x, y);
                }
            }
        }

    protected:
        static bool empty(const ColorBits i){
            return (i & 0x000000ff) == 0;
        }

        static ColorBits blend(const ColorBits src, const ColorBits dst){
            const ColorBits src_a = src & Color::a_Mask;
            if(src_a == 0) return dst;

            ColorBits dst_a = dst & Color::a_Mask;
            if(dst_a == 0) return src;
            ColorBits dst_r = dst >> Color::r_Offset & Color::a_Mask;
            ColorBits dst_g = dst >> Color::g_Offset & Color::a_Mask;
            ColorBits dst_b = dst >> Color::b_Offset & Color::a_Mask;

            dst_a -=  static_cast<ColorBits>(static_cast<float>(dst_a) * (static_cast<float>(src_a) / Color::maxValF));
            const ColorBits a = dst_a + src_a;
            dst_r = static_cast<ColorBits>(static_cast<float>(dst_r * dst_a + (src >> Color::r_Offset & Color::a_Mask) * src_a) / static_cast<ColorBits>(a));
            dst_g = static_cast<ColorBits>(static_cast<float>(dst_g * dst_a + (src >> Color::g_Offset & Color::a_Mask) * src_a) / static_cast<ColorBits>(a));
            dst_b = static_cast<ColorBits>(static_cast<float>(dst_b * dst_a + (src >> Color::b_Offset & Color::a_Mask) * src_a) / static_cast<ColorBits>(a));
            return
                dst_r << Color::r_Offset |
                dst_g << Color::g_Offset |
                dst_b << Color::b_Offset |
                    a << Color::a_Offset;
        }

    public:
        void flipY() const{
            const std::size_t rowDataCount = static_cast<std::size_t>(getWidth()) * Channels;

            for(std::size_t y = 0; y < height / 2; ++y){
                std::swap_ranges(
                    bitmapData.get() + rowDataCount * y, bitmapData.get() + rowDataCount * y + rowDataCount,
                    bitmapData.get() + rowDataCount * (height - y)
                );
            }
        }
        void blend(const int x, const int y, const Color& color) const {
            const ColorBits raw = getRaw(x, y);
            setRaw(x, y, blend(raw, color.rgba8888()));
        }

        [[nodiscard]] Pixmap crop(const int srcX, const int srcY, const int tWidth, const int tHeight) const {
            if(!valid())throw ext::RuntimeException{"Resource Released!"};
            Pixmap pixmap{tWidth, tHeight};
            pixmap.draw(*this, 0, 0, srcX, srcY, width, height);
            return pixmap;
        }

        void draw(const Pixmap& pixmap, const bool blend = true) const {
            draw(pixmap, 0, 0, pixmap.width, pixmap.height, 0, 0, pixmap.width, pixmap.height, false, blend);
        }

        void draw(const Pixmap& pixmap) const {
            draw(pixmap, 0, 0);
        }

        void draw(const Pixmap& pixmap, const int x, const int y, const bool blending = true) const {
            draw(pixmap, 0, 0, pixmap.width, pixmap.height, x, y, pixmap.width, pixmap.height, false, blending);
        }

     /** Draws an area from another Pixmap to this Pixmap. */
        void draw(const Pixmap& pixmap, const int x, const int y, const int width, const int height, const bool filtering = true, const bool blending = true) const {
            draw(pixmap, 0, 0, pixmap.width, pixmap.height, x, y, width, height, filtering, blending);
        }

     /** Draws an area from another Pixmap to this Pixmap. */
        void draw(const Pixmap& pixmap, const int x, const int y, const int srcx, const int srcy, const int srcWidth, const int srcHeight) const {
            draw(pixmap, srcx, srcy, srcWidth, srcHeight, x, y, srcWidth, srcHeight);
        }

        void draw(const Pixmap& pixmap, const int srcx, const int srcy, const int srcWidth, const int srcHeight, const int dstx, const int dsty, const int dstWidth, const int dstHeight, const bool filtering = false, const bool blending = true) const {
            const int owidth = pixmap.width;
            const int oheight = pixmap.height;

            //don't bother drawing invalid regions
            if(srcWidth == 0 || srcHeight == 0 || dstWidth == 0 || dstHeight == 0){
                return;
            }

            if(srcWidth == dstWidth && srcHeight == dstHeight){
                int sx;
                int dx;
                int sy = srcy;
                int dy = dsty;

                if(blending){
                    for(; sy < srcy + srcHeight; sy++, dy++){
                        if(sy >= oheight || dy >= height) break;

                        for(sx = srcx, dx = dstx; sx < srcx + srcWidth; sx++, dx++){
                            if(sx >= owidth || dx >= width) break;
                            setRaw(dx, dy, blend(pixmap.getRaw(sx, sy), getRaw(dx, dy)));
                        }
                    }
                }else{
                        //TODO this can be optimized with scanlines, potentially
                    for(; sy < srcy + srcHeight; sy++, dy++){
                        if(sy >= oheight || dy >= height) break;

                        for(sx = srcx, dx = dstx; sx < srcx + srcWidth; sx++, dx++){
                            if(sx >= owidth || dx >= width) break;
                            setRaw(dx, dy, pixmap.getRaw(sx, sy));
                        }
                    }
                }
            }else{
                if(filtering){
                    //blit with bilinear filtering
                    const float x_ratio = (static_cast<float>(srcWidth) - 1) / static_cast<float>(dstWidth);
                    const float y_ratio = (static_cast<float>(srcHeight) - 1) / static_cast<float>(dstHeight);
                    const int rX = Math::max(Math::round<int>(x_ratio), 1);
                    const int rY = Math::max(Math::round<int>(y_ratio), 1);
                    const int spitch = Pixmap::Channels * owidth;

                    for(int i = 0; i < dstHeight; i++){
                        const int sy = static_cast<int>(static_cast<float>(i) * y_ratio) + srcy;
                        const int dy = i + dsty;
                        const float ydiff = y_ratio * static_cast<float>(i) + static_cast<float>(srcy - sy);

                        if(sy >= oheight || dy >= height) break;

                        for(int j = 0; j < dstWidth; j++){
                            const int sx = static_cast<int>(static_cast<float>(j) * x_ratio) + srcx;
                            const int dx = j + dstx;
                            const float xdiff = x_ratio * static_cast<float>(j) + static_cast<float>(srcx - sx);
                            if(sx >= owidth || dx >= width) break;

                            const int
                                srcp = (sx + sy * owidth) * Pixmap::Channels;

                            const ColorBits
                                c1 = pixmap.getRaw(srcp),
                                c2 = sx + rX < srcWidth  ? pixmap.getRaw(srcp + Pixmap::Channels *rX) : c1,
                                c3 = sy + rY < srcHeight ? pixmap.getRaw(srcp + spitch * rY) : c1,
                                c4 = sx + rX < srcWidth && sy + rY < srcHeight ? pixmap.getRaw(srcp + Pixmap::Channels * rX + spitch * rY) : c1;

                            const float ta = (1 - xdiff) * (1 - ydiff);
                            const float tb =      xdiff  * (1 - ydiff);
                            const float tc = (1 - xdiff) *      ydiff ;
                            const float td =      xdiff  *      ydiff ;

                            const ColorBits r = Color::a_Mask & static_cast<ColorBits>(
                                static_cast<float>((c1 & Color::r_Mask) >> Color::r_Offset) * ta +
                                static_cast<float>((c2 & Color::r_Mask) >> Color::r_Offset) * tb +
                                static_cast<float>((c3 & Color::r_Mask) >> Color::r_Offset) * tc +
                                static_cast<float>((c4 & Color::r_Mask) >> Color::r_Offset) * td
                            );
                            const ColorBits g = Color::a_Mask & static_cast<ColorBits>(
                                static_cast<float>((c1 & Color::g_Mask) >> Color::g_Offset) * ta +
                                static_cast<float>((c2 & Color::g_Mask) >> Color::g_Offset) * tb +
                                static_cast<float>((c3 & Color::g_Mask) >> Color::g_Offset) * tc +
                                static_cast<float>((c4 & Color::g_Mask) >> Color::g_Offset) * td
                            );
                            const ColorBits b = Color::a_Mask & static_cast<ColorBits>(
                                static_cast<float>((c1 & Color::b_Mask) >> Color::b_Offset) * ta +
                                static_cast<float>((c2 & Color::b_Mask) >> Color::b_Offset) * tb +
                                static_cast<float>((c3 & Color::b_Mask) >> Color::b_Offset) * tc +
                                static_cast<float>((c4 & Color::b_Mask) >> Color::b_Offset) * td
                            );
                            const ColorBits a = Color::a_Mask & static_cast<ColorBits>(
                                static_cast<float>((c1 & Color::a_Mask) >> Color::a_Offset) * ta +
                                static_cast<float>((c2 & Color::a_Mask) >> Color::a_Offset) * tb +
                                static_cast<float>((c3 & Color::a_Mask) >> Color::a_Offset) * tc +
                                static_cast<float>((c4 & Color::a_Mask) >> Color::a_Offset) * td
                            );

                            const int srccol = r << Color::r_Offset | g << Color::g_Offset | b << Color::b_Offset | a << Color::a_Offset;

                            setRaw(dx, dy, !blending ? srccol : blend(srccol, getRaw(dx, dy)));
                        }
                    }
                }else{
                    //blit with nearest neighbor filtering
                    const int xratio = (srcWidth << 16) / dstWidth + 1;
                    const int yratio = (srcHeight << 16) / dstHeight + 1;

                    for(int i = 0; i < dstHeight; i++){
                        const int sy = (i * yratio >> 16) + srcy;
                        const int dy = i + dsty;
                        if(sy >= oheight || dy >= height) break;

                        for(int j = 0; j < dstWidth; j++){
                            const int sx = (j * xratio >> 16) + srcx;
                            const int dx = j + dstx;
                            if(sx >= owidth || dx >= width) break;

                            setRaw(dx, dy, !blending ? pixmap.getRaw(sx, sy) : blend(pixmap.getRaw(sx, sy), getRaw(dx, dy)));
                        }
                    }
                }
            }
        }

        //If the area of the pixmaps are the same, the longer one row it be, the faster the function works.
        void set(const Pixmap& pixmap, const int dstx, const int dsty) const {
            const int rowDataCount = pixmap.getWidth() * Channels;

            for(int y = 0; y < pixmap.getHeight(); ++y) {
                const auto indexDst =  this->dataIndex(dstx, dsty + y) * Channels;
                const auto indexSrc = pixmap.dataIndex(0   ,        y) * Channels;

                std::memcpy(bitmapData.get() + indexDst, pixmap.bitmapData.get() + indexSrc, rowDataCount);
            }
        }

        [[nodiscard]] Pixmap subMap(const int srcx, const int srcy, const int width, const int height) const{
            if(!valid())throw ext::RuntimeException{"Resource Released!"};
            if(srcx + width > this->width || srcy + height > this->height) {
                throw ext::IllegalArguments{"Sub Region Larger Than Source Pixmap!"};
            }

            const int rowDataCount = width * Channels;

            Pixmap newMap{width, height};

            for(int y = 0; y < height; ++y) {
                const auto indexDst = newMap.dataIndex(0   ,        y) * Channels;
                const auto indexSrc =  this->dataIndex(srcx, srcy + y) * Channels;

                std::memcpy(newMap.bitmapData.get() + indexDst, bitmapData.get() + indexSrc, rowDataCount);
            }

            return newMap;
        }
    };
}

