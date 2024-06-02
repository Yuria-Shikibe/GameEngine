export module UI.Selection;

export import Geom.Vector2D;
export import Geom.Rect_Orthogonal;
import ext.Concepts;
import std;

export namespace UI{
	template <Concepts::Number T>
	struct BoxSelection{
		using Point = Geom::Vector2D<T>;
		using Rect = Geom::Rect_Orthogonal<T>;

		static constexpr Point InvalidPosValue{[]{
			if constexpr (std::is_floating_point_v<T>){
				return Geom::signalNanVec2<T>;
			}else{
				return Geom::maxVec2<T>;
			}
		}()};

		Point src{InvalidPosValue};

		constexpr void setInvalid() noexcept{
			if constexpr (std::is_floating_point_v<T>){
				src.setNaN();
			}else{
				src = Geom::maxVec2<T>;
			}
		}

		constexpr void beginSelect(const typename Point::PassType srcPos) noexcept{
			src = srcPos;
		}

		/*constexpr*/ bool isSelecting() const noexcept{
			if constexpr (std::is_floating_point_v<T>){
				return !src.isNaN();
			}else{
				return src != Geom::maxVec2<T>;
			}
		}

		explicit operator bool() const noexcept{
			return isSelecting();
		}

		constexpr Rect getRegion(const typename Point::PassType curPos) const noexcept{
			return {src, curPos};
		}

		template <Concepts::Invokable<void(Rect)> Func>
		void endSelect(const typename Point::PassType curPos, Func&& func){
			func(this->getRegion(curPos));

			setInvalid();
		}
	};
}