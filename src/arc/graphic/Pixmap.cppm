module;

export module Graphic.Pixmap;

import RuntimeException;
import GL.Texture.Texture2D;
import GL.Buffer.FrameBuffer;
import OS.File;
import Image;
import Concepts;
import Graphic.Color;
import Math;
import <memory>;
import <string>;
import <iostream>;

using Graphic::Color;
using colorBits = Graphic::Color::ColorBits;

export namespace Graphic{
    class Pixmap{
    protected:
        unsigned int width{ 1 };
        unsigned int height{ 1 };
        //TODO is this necessay?
        //unsigned int bpp{ 4 };

        std::unique_ptr<unsigned char[]> data{nullptr};

    public:
        static constexpr unsigned int Channels = 4; //FOR RGBA

        [[nodiscard]] Pixmap() = default;

        [[nodiscard]] unsigned getWidth() const {
            return width;
        }

        [[nodiscard]] unsigned getHeight() const {
            return height;
        }

        // [[nodiscard]] unsigned getBpp() const {
        //     return bpp;
        // }

        [[nodiscard]] unsigned char* release() && {
            return data.release();
        }

        [[nodiscard]] unsigned char* getData() const & {
            return data.get();
        }

        [[nodiscard]] std::unique_ptr<unsigned char[]> getData() && {
            return std::move(data);
        }

        [[nodiscard]] Pixmap(const unsigned int width, const unsigned int height)
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

        Pixmap(const unsigned int width, const unsigned int height, unsigned char* data)
                    : width(width),
                      height(height) {
            this->data.reset(data);
        }

        Pixmap(const int width, const int height, unsigned char* data) : Pixmap(static_cast<unsigned int>(width), static_cast<unsigned int>(height) ,data) {

        }

        Pixmap(const Pixmap& other)
            : width(other.width),
              height(other.height),
              //bpp(other.bpp),
              data(other.copyData()) {

        }

        Pixmap(Pixmap&& other) noexcept
            : width(other.width),
              height(other.height),
              //bpp(other.bpp),
              data(std::move(other.data)) {
        }

        Pixmap& operator=(const Pixmap& other) {
            if(this == &other) return *this;
            width = other.width;
            height = other.height;
            //bpp = other.bpp;
            data = other.copyData();
            return *this;
        }

        Pixmap& operator=(Pixmap&& other) noexcept {
            if(this == &other) return *this;
            width = other.width;
            height = other.height;
            //bpp = other.bpp;
            data = std::move(other.data);
            return *this;
        }

        [[nodiscard]] bool valid() const {
            return data != nullptr && dataSize() > 0;
        }

        void create(const unsigned int width, const unsigned int height){
            data.reset(new (std::nothrow) unsigned char[width * height * Channels]{0});
            if(data == nullptr)throw ext::RuntimeException{"Failed to create pixmap!"};

            this->width = width;
            this->height = height;
        }

        void free() {
            width = height = 0;
            if(data)data.reset(nullptr);
        }

        void loadFrom(const OS::File& file) {
            unsigned b;
            const auto pixels = stbi::loadPng(file, width, height, b, Channels);

            this->data = std::unique_ptr<unsigned char[]>(pixels);
        }

        void loadFrom(const GL::Texture2D& texture2D) {
            data = texture2D.copyData();
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
                stbi::writePng(file, width, height, Channels, data.get());
            }else if(ext == ".bmp") {
                stbi::writeBmp(file, width, height, Channels, data.get());
            }
        }

        //TODO fix these two function!
        [[nodiscard]] GL::Texture2D genTex() const {
            return GL::Texture2D{width, height, copyData()};
        }

        [[nodiscard]] GL::Texture2D genTex_move() && {
            GL::Texture2D tex{width, height, std::move(data)};
            width = height = 0;
            return tex;
        }

        [[nodiscard]] constexpr auto dataIndex(const unsigned int x, const unsigned int y) const {
            if(x > width || y > height)throw ext::RuntimeException{"Array Index Out Of Bound!"};
            return (y * width + x) * Channels;
        }

        [[nodiscard]] std::unique_ptr<unsigned char[]> copyData() const {
            const auto size = dataSize();
            auto *const dataN = new unsigned char[size];
            std::memcpy(dataN, data.get(), size);

            return std::unique_ptr<unsigned char[]>(dataN);
        }

        [[nodiscard]] constexpr auto dataSize() const {
            return width * height;
        }

        void setRaw(const unsigned int x, const unsigned int y, const colorBits colorBit) const{
            auto* index = reinterpret_cast<colorBits*>(data.get() + dataIndex(x, y));
            *index = colorBit;
        }

        void set(const unsigned int x, const unsigned int y, const Graphic::Color& color) const{
            setRaw(x, y, color.argb8888());
        }

        [[nodiscard]] colorBits getRaw(const unsigned int x, const unsigned int y) const {
            const auto* index = reinterpret_cast<colorBits*>(data.get() + dataIndex(x, y));
            return *index;
        }

        [[nodiscard]] Graphic::Color get(const unsigned int x, const unsigned int y) const {
            const auto* index = data.get() + dataIndex(x, y);
            return Color{
                static_cast<float>(index[0]) / 255.0f,
                static_cast<float>(index[1]) / 255.0f,
                static_cast<float>(index[2]) / 255.0f,
                static_cast<float>(index[3]) / 255.0f
            };
        }

        [[nodiscard]] unsigned int getRaw(const size_t index) const {
            const auto* i = reinterpret_cast<colorBits*>(data.get() + index);
            return *i;
        }

        template<Concepts::Invokable<void(int, int)> T>
        void each(const T& func) {
            for(int x = 0; x < width; x++) {
                for(int y = 0; y < height; ++y) {
                    func(x, y);
                }
            }
        }

        static bool empty(const colorBits i){
            return (i & 0x000000ff) == 0;
        }

        static colorBits blend(const colorBits src, const colorBits dst){
            const colorBits src_a = src & Color::a_Mask;
            if(src_a == 0) return dst;

            colorBits dst_a = dst & Color::a_Mask;
            if(dst_a == 0) return src;
            colorBits dst_r = dst >> Color::r_Offset & Color::a_Mask;
            colorBits dst_g = dst >> Color::g_Offset & Color::a_Mask;
            colorBits dst_b = dst >> Color::b_Offset & Color::a_Mask;

            dst_a -=  static_cast<colorBits>(static_cast<float>(dst_a) * (static_cast<float>(src_a) / Color::maxValF));
            const colorBits a = dst_a + src_a;
            dst_r = static_cast<colorBits>(static_cast<float>(dst_r * dst_a + (src >> Color::r_Offset & Color::a_Mask) * src_a) / static_cast<colorBits>(a));
            dst_g = static_cast<colorBits>(static_cast<float>(dst_g * dst_a + (src >> Color::g_Offset & Color::a_Mask) * src_a) / static_cast<colorBits>(a));
            dst_b = static_cast<colorBits>(static_cast<float>(dst_b * dst_a + (src >> Color::b_Offset & Color::a_Mask) * src_a) / static_cast<colorBits>(a));
            return
                dst_r << Color::r_Offset |
                dst_g << Color::g_Offset |
                dst_b << Color::b_Offset |
                    a << Color::a_Offset;
        }

        void blend(const unsigned int x, const unsigned int y, const Color& color) const {
            const colorBits raw = getRaw(x, y);
            setRaw(x, y, blend(raw, color.rgba8888()));
        }

        [[nodiscard]] Pixmap crop(const unsigned int srcX, const unsigned int srcY, const unsigned int tWidth, const unsigned int tHeight) const {
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

        void draw(const Pixmap& pixmap, const unsigned int x, const unsigned int y, const bool blending = true) const {
            draw(pixmap, 0, 0, pixmap.width, pixmap.height, x, y, pixmap.width, pixmap.height, false, blending);
        }

     /** Draws an area from another Pixmap to this Pixmap. */
        void draw(const Pixmap& pixmap, const unsigned int x, const unsigned int y, const unsigned int width, const unsigned int height, const bool filtering = true, const bool blending = true) const {
            draw(pixmap, 0, 0, pixmap.width, pixmap.height, x, y, width, height, filtering, blending);
        }

     /** Draws an area from another Pixmap to this Pixmap. */
        void draw(const Pixmap& pixmap, const unsigned int x, const unsigned int y, const unsigned int srcx, const unsigned int srcy, const unsigned int srcWidth, const unsigned int srcHeight) const {
            draw(pixmap, srcx, srcy, srcWidth, srcHeight, x, y, srcWidth, srcHeight);
        }

        void draw(const Pixmap& pixmap, const unsigned int srcx, const unsigned int srcy, const unsigned int srcWidth, const unsigned int srcHeight, const unsigned int dstx, const unsigned int dsty, const unsigned int dstWidth, const unsigned int dstHeight, const bool filtering = false, const bool blending = true) const {
            const unsigned int owidth = pixmap.width;
            const unsigned int oheight = pixmap.height;

            //don't bother drawing invalid regions
            if(srcWidth == 0 || srcHeight == 0 || dstWidth == 0 || dstHeight == 0){
                return;
            }

            if(srcWidth == dstWidth && srcHeight == dstHeight){
                unsigned int sx;
                unsigned int dx;
                unsigned int sy = srcy;
                unsigned int dy = dsty;

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
                    const unsigned int rX = std::max(Math::round<unsigned>(x_ratio), 1u);
                    const unsigned int rY = std::max(Math::round<unsigned>(y_ratio), 1u);
                    const unsigned int spitch = Pixmap::Channels * owidth;

                    for(unsigned int i = 0; i < dstHeight; i++){
                        const unsigned int sy = static_cast<unsigned int>(static_cast<float>(i) * y_ratio) + srcy;
                        const unsigned int dy = i + dsty;
                        const float ydiff = y_ratio * static_cast<float>(i) + static_cast<float>(srcy - sy);

                        if(sy >= oheight || dy >= height) break;

                        for(unsigned int j = 0; j < dstWidth; j++){
                            const unsigned int sx = static_cast<unsigned int>(static_cast<float>(j) * x_ratio) + srcx;
                            const unsigned int dx = j + dstx;
                            const float xdiff = x_ratio * static_cast<float>(j) + static_cast<float>(srcx - sx);
                            if(sx >= owidth || dx >= width) break;

                            const colorBits
                                srcp = (sx + sy * owidth) * Pixmap::Channels,
                                c1 = pixmap.getRaw(srcp),
                                c2 = sx + rX < srcWidth  ? pixmap.getRaw(srcp + Pixmap::Channels *rX) : c1,
                                c3 = sy + rY < srcHeight ? pixmap.getRaw(srcp + spitch * rY) : c1,
                                c4 = sx + rX < srcWidth && sy + rY < srcHeight ? pixmap.getRaw(srcp + Pixmap::Channels * rX + spitch * rY) : c1;

                            const float ta = (1 - xdiff) * (1 - ydiff);
                            const float tb =      xdiff  * (1 - ydiff);
                            const float tc = (1 - xdiff) *      ydiff ;
                            const float td =      xdiff  *      ydiff ;

                            const colorBits r = Color::a_Mask & static_cast<colorBits>(
                                static_cast<float>((c1 & Color::r_Mask) >> Color::r_Offset) * ta +
                                static_cast<float>((c2 & Color::r_Mask) >> Color::r_Offset) * tb +
                                static_cast<float>((c3 & Color::r_Mask) >> Color::r_Offset) * tc +
                                static_cast<float>((c4 & Color::r_Mask) >> Color::r_Offset) * td
                            );
                            const colorBits g = Color::a_Mask & static_cast<colorBits>(
                                static_cast<float>((c1 & Color::g_Mask) >> Color::g_Offset) * ta +
                                static_cast<float>((c2 & Color::g_Mask) >> Color::g_Offset) * tb +
                                static_cast<float>((c3 & Color::g_Mask) >> Color::g_Offset) * tc +
                                static_cast<float>((c4 & Color::g_Mask) >> Color::g_Offset) * td
                            );
                            const colorBits b = Color::a_Mask & static_cast<colorBits>(
                                static_cast<float>((c1 & Color::b_Mask) >> Color::b_Offset) * ta +
                                static_cast<float>((c2 & Color::b_Mask) >> Color::b_Offset) * tb +
                                static_cast<float>((c3 & Color::b_Mask) >> Color::b_Offset) * tc +
                                static_cast<float>((c4 & Color::b_Mask) >> Color::b_Offset) * td
                            );
                            const colorBits a = Color::a_Mask & static_cast<colorBits>(
                                static_cast<float>((c1 & Color::a_Mask) >> Color::a_Offset) * ta +
                                static_cast<float>((c2 & Color::a_Mask) >> Color::a_Offset) * tb +
                                static_cast<float>((c3 & Color::a_Mask) >> Color::a_Offset) * tc +
                                static_cast<float>((c4 & Color::a_Mask) >> Color::a_Offset) * td
                            );

                            const unsigned int srccol = r << Color::r_Offset | g << Color::g_Offset | b << Color::b_Offset | a << Color::a_Offset;

                            setRaw(dx, dy, !blending ? srccol : blend(srccol, getRaw(dx, dy)));
                        }
                    }
                }else{
                    //blit with nearest neighbor filtering
                    const unsigned int xratio = (srcWidth << 16) / dstWidth + 1;
                    const unsigned int yratio = (srcHeight << 16) / dstHeight + 1;

                    for(unsigned int i = 0; i < dstHeight; i++){
                        const unsigned int sy = (i * yratio >> 16) + srcy;
                        const unsigned int dy = i + dsty;
                        if(sy >= oheight || dy >= height) break;

                        for(unsigned int j = 0; j < dstWidth; j++){
                            const unsigned int sx = (j * xratio >> 16) + srcx;
                            const unsigned int dx = j + dstx;
                            if(sx >= owidth || dx >= width) break;

                            setRaw(dx, dy, !blending ? pixmap.getRaw(sx, sy) : blend(pixmap.getRaw(sx, sy), getRaw(dx, dy)));
                        }
                    }
                }
            }
        }

        //If the area of the pixmaps are the same, the longer one row it be, the faster the function works.
        void set(const Pixmap& pixmap, const unsigned int dstx, const unsigned int dsty) const {
            const unsigned int rowDataCount = pixmap.getWidth() * Channels;

            for(unsigned int y = 0; y < pixmap.getHeight(); ++y) {
                const auto indexDst =  this->dataIndex(dstx, dsty + y);
                const auto indexSrc = pixmap.dataIndex(0   ,        y);

                std::memcpy(data.get() + indexDst, pixmap.data.get() + indexSrc, rowDataCount);
            }
        }

        [[nodiscard]] Pixmap subMap(const unsigned int srcx, const unsigned int srcy, const unsigned int width, const unsigned int height) const{
            if(!valid())throw ext::RuntimeException{"Resource Released!"};
            if(srcx + width > this->width || srcy + height > this->height) {
                throw ext::IllegalArguments{"Sub Region Larger Than Source Pixmap!"};
            }

            const unsigned int rowDataCount = width * Channels;

            Pixmap newMap{width, height};

            for(unsigned int y = 0; y < height; ++y) {
                const auto indexDst = newMap.dataIndex(0   ,        y);
                const auto indexSrc =  this->dataIndex(srcx, srcy + y);

                std::memcpy(newMap.data.get() + indexDst, data.get() + indexSrc, rowDataCount);
            }

            return newMap;
        }
    };
}

