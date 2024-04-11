//
// Created by Matrix on 2024/3/6.
//

export module Graphic.Trail;

import std;
import Geom.Vector2D;
import Math;
import Concepts;

export namespace Graphic{
    struct Trail {
        // static constexpr float minSpacingSqr = 36;
        using NodeType = std::pair<Geom::Vec2, float>;

    protected:
        std::deque<NodeType> points{};
        Geom::Vec2 lastPos{};
        float lastAngle{-1};
        float lastwidth{};
        size_t length{};

    public:
        explicit Trail(const size_t length)
            : length(length){

        }

        [[nodiscard]] float getWidth() const{
            return lastwidth;
        }

        /** Adds a new point to the trail at intervals. */
        void update(const float x, const float y, float width = 1.0f){
            // if(lastPos.dst2(x, y) < minSpacingSqr)return;

            if(points.size() > maxSize()){
                points.pop_front();
            }

            points.emplace_back(Geom::Vec2{x, y}, width);

            //update last position regardless, so it joins
            lastAngle = Math::angleRad(x - lastPos.x, y - lastPos.y);
            lastPos.set(x, y);
            lastwidth = width;
        }

        void shorten(){
            if(!points.empty()){
                points.pop_front();
            }
        }

        void clear(){
            points.clear();
        }

        [[nodiscard]] size_t size() const{
            return points.size();
        }

        [[nodiscard]] size_t maxSize() const{
            return length;
        }

        void resize(const size_t length){
            this->length = length;
        }

        template <typename Func, bool requiresProgress = true>
        requires
            (requiresProgress && Concepts::Invokable<Func, void(
                Geom::Vec2, Geom::Vec2, Geom::Vec2, Geom::Vec2, //P1 P2 P3 P4
                Math::Progress, Math::Progress //Progress1, Progress2
            )>)  || (!requiresProgress && Concepts::Invokable<Func, void(
                Geom::Vec2, Geom::Vec2, Geom::Vec2, Geom::Vec2//P1 P2 P3 P4
            )>)
        void each(const float width, Func&& comsumer) const{
            float lastAngle = this->lastAngle;
            const size_t size = this->size();
            const float drawSize = width / static_cast<float>(size);

            for(int i = 0; i < size; ++i){
                auto [pos1, w1] = points[i];
                float w2{0};
                Geom::Vec2 pos2{};

                //last position is always lastX/Y/W
                if(i < size - 1){
                    std::tie(pos2, w2) = points[i + 1];
                }else{
                    pos2 = lastPos;
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