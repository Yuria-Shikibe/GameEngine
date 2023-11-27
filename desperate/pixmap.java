//
// public class Pixmap implements Disposable{
//
//
//     public void replace(IntIntf func){
//         for(int x = 0; x < width; x++){
//             for(int y = 0; y < height; y++){
//                 setRaw(x, y, func.get(getRaw(x, y)));
//             }
//         }
//     }
//
//     /** Fills the complete bitmap with the specified color. */
//     public void fill(int color){
//         int len = width * height * 4;
//         for(int i = 0; i < len; i += 4){
//             pixels.putInt(i, color);
//         }
//     }
//
//     /** Fills the complete bitmap with the specified color. */
//     public void fill(Color color){
//         fill(color.rgba());
//     }
//
//     /** @return whether this point is in the pixmap. */
//     public boolean in(int x, int y){
//         return x >= 0 && y >= 0 && x < width && y < height;
//     }
//
//     public Pixmap crop(int x, int y, int width, int height){
//         if(isDisposed()) throw new IllegalStateException("input is disposed.");
//         Pixmap pixmap = new Pixmap(width, height);
//         pixmap.draw(this, 0, 0, x, y, width, height);
//         return pixmap;
//     }
//
//     /** @return a newly allocated pixmap, flipped vertically. */
//     public Pixmap flipY(){
//         Pixmap copy = new Pixmap(width, height);
//
//         //TODO this can be optimized significantly by putting each line
//         for(int x = 0; x < width; x++){
//             for(int y = 0; y < height; y++){
//                 copy.setRaw(x, height - 1 - y, getRaw(x, y));
//             }
//         }
//
//         return copy;
//     }
//
//     /** @return a newly allocated pixmap, flipped horizontally. */
//     public Pixmap flipX(){
//         Pixmap copy = new Pixmap(width, height);
//
//         for(int x = 0; x < width; x++){
//             for(int y = 0; y < height; y++){
//                 copy.set(width - 1 - x, y, getRaw(x, y));
//             }
//         }
//
//         return copy;
//     }
//
//     /** @return a newly allocated pixmap with the specified outline. */
//     public Pixmap outline(Color color, int radius){
//         return outline(color.rgba(), radius);
//     }
//
//     /** @return a newly allocated pixmap with the specified outline. */
//     public Pixmap outline(int color, int radius){
//         Pixmap pixmap = copy();
//
//         //TODO this messes with antialiasing?
//         for(int x = 0; x < width; x++){
//             for(int y = 0; y < height; y++){
//                 if(getA(x, y) == 0){
//                     boolean found = false;
//                     outer:
//                     for(int dx = -radius; dx <= radius; dx++){
//                         for(int dy = -radius; dy <= radius; dy++){
//                             if((dx*dx + dy*dy <= radius*radius) && !empty(get(x + dx, y + dy))){
//                                 found = true;
//                                 break outer;
//                             }
//                         }
//                     }
//                     if(found){
//                         pixmap.setRaw(x, y, color);
//                     }
//                 }
//             }
//         }
//         return pixmap;
//     }
//
//     /** Draws a line between the given coordinates using the provided RGBA color. */
//     public void drawLine(int x, int y, int x2, int y2, int color){
//         int dy = y - y2;
//         int dx = x - x2;
//         int fraction, stepx, stepy;
//
//         if(dy < 0){
//             dy = -dy;
//             stepy = -1;
//         }else{
//             stepy = 1;
//         }
//         if(dx < 0){
//             dx = -dx;
//             stepx = -1;
//         }else{
//             stepx = 1;
//         }
//         dy <<= 1;
//         dx <<= 1;
//
//         set(x, y, color);
//
//         if(dx > dy){
//             fraction = dy - (dx >> 1);
//             while(x != x2){
//                 if(fraction >= 0){
//                     y += stepy;
//                     fraction -= dx;
//                 }
//                 x += stepx;
//                 fraction += dy;
//                 set(x, y, color);
//             }
//         }else{
//             fraction = dx - (dy >> 1);
//             while(y != y2){
//                 if(fraction >= 0){
//                     x += stepx;
//                     fraction -= dy;
//                 }
//                 y += stepy;
//                 fraction += dx;
//                 set(x, y, color);
//             }
//         }
//     }
//
//     /** Draws a rectangle outline starting at x, y extending by width to the right and by height downwards (y-axis points downwards) using the provided color. */
//     public void drawRect(int x, int y, int width, int height, int color){
//         hline(x, x + width - 1, y, color);
//         hline(x, x + width - 1, y + height - 1, color);
//         vline(y, y + height - 1, x, color);
//         vline(y, y + height - 1, x + width - 1, color);
//     }
//
//     void circlePoints(int cx, int cy, int x, int y, int col){
//         if(x == 0){
//             set(cx, cy + y, col);
//             set(cx, cy - y, col);
//             set(cx + y, cy, col);
//             set(cx - y, cy, col);
//         }else if(x == y){
//             set(cx + x, cy + y, col);
//             set(cx - x, cy + y, col);
//             set(cx + x, cy - y, col);
//             set(cx - x, cy - y, col);
//         }else if(x < y){
//             set(cx + x, cy + y, col);
//             set(cx - x, cy + y, col);
//             set(cx + x, cy - y, col);
//             set(cx - x, cy - y, col);
//             set(cx + y, cy + x, col);
//             set(cx - y, cy + x, col);
//             set(cx + y, cy - x, col);
//             set(cx - y, cy - x, col);
//         }
//     }
//
//     public void drawCircle(int x, int y, int radius, int col){
//         int px = 0;
//         int py = radius;
//         int p = (5 - radius * 4) / 4;
//
//         circlePoints(x, y, px, py, col);
//         while(px < py){
//             px++;
//             if(p < 0){
//                 p += 2 * px + 1;
//             }else{
//                 py--;
//                 p += 2 * (px - py) + 1;
//             }
//             circlePoints(x, y, px, py, col);
//         }
//     }
//
//     public void draw(PixmapRegion region){
//         draw(region.pixmap, region.x, region.y, region.width, region.height, 0, 0, region.width, region.height);
//     }
//
//     public void draw(PixmapRegion region, boolean blend){
//         draw(region.pixmap, region.x, region.y, region.width, region.height, 0, 0, region.width, region.height, false, blend);
//     }
//
//     public void draw(PixmapRegion region, int x, int y){
//         draw(region.pixmap, region.x, region.y, region.width, region.height, x, y, region.width, region.height);
//     }
//
//     public void draw(PixmapRegion region, int x, int y, int width, int height){
//         draw(region.pixmap, region.x, region.y, region.width, region.height, x, y, width, height);
//     }
//
//     public void draw(PixmapRegion region, int x, int y, int srcx, int srcy, int srcWidth, int srcHeight){
//         draw(region.pixmap, x, y, region.x + srcx, region.y + srcy, srcWidth, srcHeight);
//     }
//
//     public void draw(PixmapRegion region, int srcx, int srcy, int srcWidth, int srcHeight, int dstx, int dsty, int dstWidth, int dstHeight){
//         draw(region.pixmap, region.x + srcx, region.y + srcy, srcWidth, srcHeight, dstx, dsty, dstWidth, dstHeight);
//     }
//
//
//     /**
//      * Draws an area from another Pixmap to this Pixmap. This will automatically scale and stretch the source image to the
//      * specified target rectangle. Blending is currently unsupported for stretched/scaled pixmaps.
//      * @param pixmap The other Pixmap
//      * @param srcx The source x-coordinate (top left corner)
//      * @param srcy The source y-coordinate (top left corner);
//      * @param srcWidth The width of the area from the other Pixmap in pixels
//      * @param srcHeight The height of the area from the other Pixmap in pixels
//      * @param dstx The target x-coordinate (top left corner)
//      * @param dsty The target y-coordinate (top left corner)
//      * @param dstWidth The target width
//      * @param dstHeight the target height
//      */
//     public void draw(Pixmap pixmap, int srcx, int srcy, int srcWidth, int srcHeight, int dstx, int dsty, int dstWidth, int dstHeight, boolean filtering, boolean blending){
//         int width = this.width, height = this.height, owidth = pixmap.width, oheight = pixmap.height;
//
//         //don't bother drawing invalid regions
//         if(srcWidth == 0 || srcHeight == 0 || dstWidth == 0 || dstHeight == 0){
//             return;
//         }
//
//         if(srcWidth == dstWidth && srcHeight == dstHeight){
//
//             //same-size blit, no filtering
//             int sx, dx;
//             int sy = srcy, dy = dsty;
//
//             if(blending){
//                 for(; sy < srcy + srcHeight; sy++, dy++){
//                     if(sy < 0 || dy < 0) continue;
//                     if(sy >= oheight || dy >= height) break;
//
//                     for(sx = srcx, dx = dstx; sx < srcx + srcWidth; sx++, dx++){
//                         if(sx < 0 || dx < 0) continue;
//                         if(sx >= owidth || dx >= width) break;
//                         setRaw(dx, dy, blend(pixmap.getRaw(sx, sy), getRaw(dx, dy)));
//                     }
//                 }
//             }else{
//                 //TODO this can be optimized with scanlines, potentially
//                 for(; sy < srcy + srcHeight; sy++, dy++){
//                     if(sy < 0 || dy < 0) continue;
//                     if(sy >= oheight || dy >= height) break;
//
//                     for(sx = srcx, dx = dstx; sx < srcx + srcWidth; sx++, dx++){
//                         if(sx < 0 || dx < 0) continue;
//                         if(sx >= owidth || dx >= width) break;
//                         setRaw(dx, dy, pixmap.getRaw(sx, sy));
//                     }
//                 }
//             }
//         }else{
//             if(filtering){
//                 //blit with bilinear filtering
//                 float x_ratio = ((float)srcWidth - 1) / dstWidth;
//                 float y_ratio = ((float)srcHeight - 1) / dstHeight;
//                 int rX = Math.max(Mathf.round(x_ratio), 1), rY = Math.max(Mathf.round(y_ratio), 1);
//                 float xdiff, ydiff;
//                 int spitch = 4 * owidth;
//                 int dx, dy, sx, sy, i = 0, j;
//                 ByteBuffer spixels = pixmap.pixels;
//
//                 for(; i < dstHeight; i++){
//                     sy = (int)(i * y_ratio) + srcy;
//                     dy = i + dsty;
//                     ydiff = (y_ratio * i + srcy) - sy;
//                     if(sy < 0 || dy < 0) continue;
//                     if(sy >= oheight || dy >= height) break;
//
//                     for(j = 0; j < dstWidth; j++){
//                         sx = (int)(j * x_ratio) + srcx;
//                         dx = j + dstx;
//                         xdiff = (x_ratio * j + srcx) - sx;
//                         if(sx < 0 || dx < 0) continue;
//                         if(sx >= owidth || dx >= width) break;
//
//                         int
//                         srcp = (sx + sy * owidth) * 4,
//                         c1 = spixels.getInt(srcp),
//                         c2 = sx + rX < srcWidth ? spixels.getInt(srcp + 4 *rX) : c1,
//                         c3 = sy + rY < srcHeight ? spixels.getInt(srcp + spitch * rY) : c1,
//                         c4 = sx + rX < srcWidth && sy + rY < srcHeight ? spixels.getInt(srcp + 4 * rX + spitch * rY) : c1;
//
//                         float ta = (1 - xdiff) * (1 - ydiff);
//                         float tb = (xdiff) * (1 - ydiff);
//                         float tc = (1 - xdiff) * (ydiff);
//                         float td = (xdiff) * (ydiff);
//
//                         int r = (int)(((c1 & 0xff000000) >>> 24) * ta + ((c2 & 0xff000000) >>> 24) * tb + ((c3 & 0xff000000) >>> 24) * tc + ((c4 & 0xff000000) >>> 24) * td) & 0xff;
//                         int g = (int)(((c1 & 0xff0000) >>> 16) * ta + ((c2 & 0xff0000) >>> 16) * tb + ((c3 & 0xff0000) >>> 16) * tc + ((c4 & 0xff0000) >>> 16) * td) & 0xff;
//                         int b = (int)(((c1 & 0xff00) >>> 8) * ta + ((c2 & 0xff00) >>> 8) * tb + ((c3 & 0xff00) >>> 8) * tc + ((c4 & 0xff00) >>> 8) * td) & 0xff;
//                         int a = (int)((c1 & 0xff) * ta + (c2 & 0xff) * tb + (c3 & 0xff) * tc + (c4 & 0xff) * td) & 0xff;
//                         int srccol = (r << 24) | (g << 16) | (b << 8) | a;
//
//                         setRaw(dx, dy, !blending ? srccol : blend(srccol, getRaw(dx, dy)));
//                     }
//                 }
//             }else{
//                 //blit with nearest neighbor filtering
//                 int xratio = (srcWidth << 16) / dstWidth + 1;
//                 int yratio = (srcHeight << 16) / dstHeight + 1;
//                 int dx, dy, sx, sy;
//
//                 for(int i = 0; i < dstHeight; i++){
//                     sy = ((i * yratio) >> 16) + srcy;
//                     dy = i + dsty;
//                     if(sy < 0 || dy < 0) continue;
//                     if(sy >= oheight || dy >= height) break;
//
//                     for(int j = 0; j < dstWidth; j++){
//                         sx = ((j * xratio) >> 16) + srcx;
//                         dx = j + dstx;
//                         if(sx < 0 || dx < 0) continue;
//                         if(sx >= owidth || dx >= width) break;
//
//                         setRaw(dx, dy, !blending ? pixmap.getRaw(sx, sy) : blend(pixmap.getRaw(sx, sy), getRaw(dx, dy)));
//                     }
//                 }
//             }
//         }
//     }
//
//     /**
//      * Fills a rectangle starting at x, y extending by width to the right and by height downwards (y-axis points downwards) using
//      * the current color.
//      * @param x The x coordinate
//      * @param y The y coordinate
//      * @param width The width in pixels
//      * @param height The height in pixels
//      */
//     public void fillRect(int x, int y, int width, int height, int color){
//         int x2 = x + width - 1;
//         int y2 = y + height - 1;
//
//         if(x >= this.width) return;
//         if(y >= this.height) return;
//         if(x2 < 0) return;
//         if(y2 < 0) return;
//         if(x < 0) x = 0;
//         if(y < 0) y = 0;
//         if(x2 >= this.width) x2 = this.width - 1;
//         if(y2 >= this.height) y2 = this.height - 1;
//
//         y2++;
//         while(y != y2){
//             hline(x, x2, y, color);
//             y++;
//         }
//     }
//
//     /**
//      * Fills a circle with the center at x,y and a radius using the current color.
//      * @param radius The radius in pixels
//      */
//     public void fillCircle(int x, int y, int radius, int color){
//         int f = 1 - radius;
//         int ddF_x = 1;
//         int ddF_y = -2 * radius;
//         int px = 0;
//         int py = radius;
//
//         hline(x, x, y + radius, color);
//         hline(x, x, y - radius, color);
//         hline(x - radius, x + radius, y, color);
//
//         while(px < py){
//             if(f >= 0){
//                 py--;
//                 ddF_y += 2;
//                 f += ddF_y;
//             }
//             px++;
//             ddF_x += 2;
//             f += ddF_x;
//             hline(x - px, x + px, y + py, color);
//             hline(x - px, x + px, y - py, color);
//             hline(x - py, x + py, y + px, color);
//             hline(x - py, x + py, y - px, color);
//         }
//     }
//
//     /** @return The pixel color in RGBA8888 format, or 0 if out of bounds. */
//     public int get(int x, int y){
//         return in(x, y) ? pixels.getInt((x + y * width) * 4) : 0;
//     }
//
//     /** @return The pixel color in RGBA8888 format. No bounds checks are done! */
//     public int getRaw(int x, int y){
//         return pixels.getInt((x + y * width) * 4);
//     }
//
//     /** @return The pixel alpha as a byte, 0-255. No bounds checks are done! */
//     public int getA(int x, int y){
//         return pixels.get((x + y * width) * 4 + 3) & 0xff;
//     }
//
//     /** @return whether the alpha at a position is 0. No bounds checks are done. */
//     public boolean empty(int x, int y){
//         return pixels.get((x + y * width) * 4 + 3) == 0;
//     }
//
//     /** @return The width of the Pixmap in pixels. */
//     public int getWidth(){
//         return width;
//     }
//
//     /** @return The height of the Pixmap in pixels. */
//     public int getHeight(){
//         return height;
//     }
//
//     /** Releases all resources associated with this Pixmap. */
//     @Override
//     public void dispose(){
//         if(handle <= 0) return;
//         free(handle);
//         handle = 0;
//     }
//
//     @Override
//     public boolean isDisposed(){
//         return handle == 0;
//     }
//
//     public void set(int x, int y, Color color){
//         set(x, y, color.rgba());
//     }
//
//     /**
//      * Sets a pixel at the given location with the given color.
//      * @param color the color in RGBA8888 format.
//      */
//     public void set(int x, int y, int color){
//         if(in(x, y)){
//             pixels.putInt((x + y * width) * 4, color);
//         }
//     }
//
//     /**
//      * Sets a pixel at the given location with the given color. No bounds checks are done!
//      * @param color the color in RGBA8888 format.
//      */
//     public void setRaw(int x, int y, int color){
//         pixels.putInt((x + y * width) * 4, color);
//     }
//
//     /**
//      * Returns the OpenGL ES format of this Pixmap. Used as the seventh parameter to
//      * {@link GL20#glTexImage2D(int, int, int, int, int, int, int, int, java.nio.Buffer)}.
//      * @return GL_RGBA
//      */
//     public int getGLFormat(){
//         return Gl.rgba;
//     }
//
//     /**
//      * Returns the OpenGL ES format of this Pixmap. Used as the third parameter to
//      * {@link GL20#glTexImage2D(int, int, int, int, int, int, int, int, java.nio.Buffer)}.
//      * @return GL_RGBA
//      */
//     public int getGLInternalFormat(){
//         return Gl.rgba;
//     }
//
//     /**
//      * Returns the OpenGL ES type of this Pixmap. Used as the eighth parameter to
//      * {@link GL20#glTexImage2D(int, int, int, int, int, int, int, int, java.nio.Buffer)}.
//      * @return GL_UNSIGNED_BYTE
//      */
//     public int getGLType(){
//         return Gl.unsignedByte;
//     }
//
//     /** @return the direct {@link ByteBuffer} holding the pixel data. */
//     public ByteBuffer getPixels(){
//         if(handle == 0) throw new ArcRuntimeException("Pixmap already disposed");
//         return pixels;
//     }
//
//     void hline(int x1, int x2, int y, int color){
//         if(y < 0 || y >= height) return;
//         int tmp;
//
//         if(x1 > x2){
//             tmp = x1;
//             x1 = x2;
//             x2 = tmp;
//         }
//
//         if(x1 >= width) return;
//         if(x2 < 0) return;
//
//         if(x1 < 0) x1 = 0;
//         if(x2 >= width) x2 = width - 1;
//         x2++;
//
//         while(x1 != x2){
//             setRaw(x1++, y, color);
//         }
//     }
//
//     void vline(int y1, int y2, int x, int color){
//         if(x < 0 || x >= width) return;
//         int tmp;
//
//         if(y1 > y2){
//             tmp = y1;
//             y1 = y2;
//             y2 = tmp;
//         }
//
//         if(y1 >= height) return;
//         if(y2 < 0) return;
//
//         if(y1 < 0) y1 = 0;
//         if(y2 >= height) y2 = height - 1;
//         y2++;
//
//         while(y1 != y2){
//             setRaw(x, y1++, color);
//         }
//     }
//
//     /** @return the blended result of the input colors */
//     public static int blend(int src, int dst){
//         int src_a = src & 0xff;
//         if(src_a == 0) return dst;
//
//         int dst_a = dst & 0xff;
//         if(dst_a == 0) return src;
//         int dst_b = (dst >>> 8) & 0xff;
//         int dst_g = (dst >>> 16) & 0xff;
//         int dst_r = (dst >>> 24) & 0xff;
//
//         dst_a -= (dst_a * src_a) / 255;
//         int a = dst_a + src_a;
//         dst_r = (dst_r * dst_a + ((src >>> 24) & 0xff) * src_a) / a;
//         dst_g = (dst_g * dst_a + ((src >>> 16) & 0xff) * src_a) / a;
//         dst_b = (dst_b * dst_a + ((src >>> 8) & 0xff) * src_a) / a;
//         return ((dst_r << 24) | (dst_g << 16) | (dst_b << 8) | a);
//     }
//
//     /**
//      * Different pixel formats.
//      * @author mzechner
//      */
//     public enum Format{
//         alpha(Gl.unsignedByte, Gl.alpha),
//         intensity(Gl.unsignedByte, Gl.alpha),
//         luminanceAlpha(Gl.unsignedByte, Gl.luminanceAlpha),
//         rgb565(Gl.unsignedShort565, Gl.rgb),
//         rgba4444(Gl.unsignedShort4444, Gl.rgba),
//         rgb888(Gl.unsignedByte, Gl.rgb),
//         rgba8888(Gl.unsignedByte, Gl.rgba);
//
//         public static final Format[] all = values();
//         public final int glFormat, glType;
//
//         Format(int glType, int glFormat){
//             this.glFormat = glFormat;
//             this.glType = glType;
//         }
//     }
//
//     /*JNI
//
//     #include <stdlib.h>
//     #include <stdint.h>
//
//     #include <stdlib.h>
//     #define STB_IMAGE_IMPLEMENTATION
//     #define STBI_FAILURE_USERMSG
//     #define STBI_NO_STDIO
//     #ifdef __APPLE__
//     #define STBI_NO_THREAD_LOCALS
//     #endif
//     #include "stb_image.h"
//
//     */
// }