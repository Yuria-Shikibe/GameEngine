//
// Created by Matrix on 2024/3/6.
//

export module Graphic.Trail;

import std;
import Geom.Vector2D;
import Geom.Rect_Orthogonal;
import Math;
import ext.Concepts;
import Graphic.Color;

export namespace Graphic{
    struct Trail {
        // static constexpr float minSpacingSqr = 36;
        // [pos, width scl]
        using NodeType = std::pair<Geom::Vec2, float>;
        using DataType = std::deque<NodeType>;

    protected:
        DataType points{};
        Geom::Vec2 latestPos{};
        Geom::Vec2 earliestPos{};
        float lastAngle{-1};
        float lastwidth{};
        std::size_t length{};

    public:
        /**
         * @return Distance between head and tail
         */
        float getDst() const noexcept{
            return latestPos.dst(earliestPos);
        }

        /**
         * @brief Not accurate, but enough
         */
        Geom::OrthoRectFloat getBound() const noexcept{
            return Geom::OrthoRectFloat{}.setVert(latestPos, earliestPos);
        }

        struct DefDraw{
            Graphic::Color src;
            void operator()(Geom::Vec2, Geom::Vec2, Geom::Vec2, Geom::Vec2) const;
        };

        struct DefDraw_WithLerp{
            Graphic::Color src, dst;
            void operator()(Geom::Vec2, Geom::Vec2, Geom::Vec2, Geom::Vec2, //P1 P2 P3 P4
                Math::Progress, Math::Progress) const;
        };

        [[nodiscard]] Trail() = default;

        explicit Trail(const std::size_t length)
            : length(length){
        }

        [[nodiscard]] float getWidth() const{
            return lastwidth;
        }

        /** Adds a new point to the trail at intervals. */
        void update(const float x, const float y, float widthScl = 1.0f){
            // if(lastPos.dst2(x, y) < minSpacingSqr)return;

            if(points.size() > maxSize()){
                points.pop_front();
            }

            points.emplace_back(Geom::Vec2{x, y}, widthScl);

            earliestPos = points.front().first;

            //update last position regardless, so it joins
            lastAngle = Math::angleRad(x - latestPos.x, y - latestPos.y);
            latestPos.set(x, y);
            lastwidth = widthScl;
        }

        void shorten(){
            if(!points.empty()){
                points.pop_front();
            }
        }

        DataType getPoints() && noexcept{
            return std::move(points);
        }

        void clear() noexcept{
            points.clear();
        }

        [[nodiscard]] std::size_t size() const noexcept{
            return points.size();
        }

        [[nodiscard]] std::size_t maxSize() const noexcept{
            return length;
        }

        void resize(const std::size_t length) noexcept{
            this->length = length;
        }

        [[nodiscard]] Geom::Vec2 getLastPos() const noexcept{ return latestPos; }

        template <typename Func, bool requiresProgress = true>
        requires
            (requiresProgress && Concepts::Invokable<Func, void(
                Geom::Vec2, Geom::Vec2, Geom::Vec2, Geom::Vec2, //P1 P2 P3 P4
                Math::Progress, Math::Progress //Progress1, Progress2
            )>)  || (!requiresProgress && Concepts::Invokable<Func, void(
                Geom::Vec2, Geom::Vec2, Geom::Vec2, Geom::Vec2//P1 P2 P3 P4
            )>)
        void each(const float width, Func&& comsumer, const std::size_t drop = 0) const{
            auto subRange = std::ranges::subrange{points.begin() + drop, points.end()};

            float lastAngle = this->lastAngle;
            const std::size_t size = subRange.size();
            const float drawSize = width / static_cast<float>(size);

            for(int i = 0; i < subRange.size(); ++i){
                auto [pos1, w1] = subRange[i];
                float w2{0};
                Geom::Vec2 pos2{};

                //last position is always lastX/Y/W
                if(i < size - 1){
                    std::tie(pos2, w2) = subRange[i + 1];
                }else{
                    pos2 = latestPos;
                    w2 = lastwidth;
                }

                const float z2 = -Math::angleRad(pos2.x - pos1.x, pos2.y - pos1.y);
                //end of the trail (i = 0) has the same angle as the next.
                const float z1 = i == 0 ? z2 : lastAngle;

                if(w1 <= 0.0001f || w2 <= 0.0001f) continue;

                const Geom::Vec2 c = Geom::Vec2::byPolarRad(Math::HALF_PI - z1, drawSize * w1 * static_cast<float>(i));
                const Geom::Vec2 n = Geom::Vec2::byPolarRad(Math::HALF_PI - z2, drawSize * w1 * static_cast<float>(i + 1));

                if constexpr (requiresProgress){
                    float progressFormer = static_cast<float>(i) / static_cast<float>(size);
                    float progressLatter = static_cast<float>(i + 1) / static_cast<float>(size);

                    comsumer(pos1 - c, pos1 + c, pos2 + n, pos2 - n, progressFormer, progressLatter);
                }else{
                    comsumer(pos1 - c, pos1 + c, pos2 + n, pos2 - n);
                }


                lastAngle = z2;
            }
        }

        /**
         * @tparam Func void(CapPos, size, angle)
         */
        template <Concepts::Invokable<void(
            Geom::Vec2, float, float //P1 size angle
        )> Func>
        void cap(const float width, Func&& comsumer) const{
             if(size() > 0){
                 const auto size = this->size();
                 auto [pos, w] = points.back();
                 w = w * width * (1.0f - 1.0f / static_cast<float>(size)) * 2.0f;
                 if(w <= 0.001f) return;

                 comsumer(pos, w, -lastAngle * Math::RADIANS_TO_DEGREES);
             }
         }
    };
}
