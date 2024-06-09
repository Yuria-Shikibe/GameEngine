export module Math.Algo.StripPacker2D;


#if DEBUG_CHECK
import ext.RuntimeException;
#endif

import std;
import ext.Concepts;
import Geom.Rect_Orthogonal;
import Geom.Vector2D;
import Math;

export namespace Math {
	/**
     * @brief
     * @tparam T Should Avoid Copy When Tgt Type is provided
     * @tparam N pack arithmetic type
     * @tparam trans Trans the Tgt to a non-temp rect
     */
    template <typename T, Concepts::Number N, auto trans>
		requires requires(){
    		requires std::is_pointer_v<T>;
    		std::same_as<Geom::Rect_Orthogonal<N>&, std::invoke_result_t<decltype(trans), T>>;
	    }
	struct StripPacker2D {
    protected:
		using Rect = Geom::Rect_Orthogonal<N>;
    	using SubRectArr = std::array<Rect, 3>;

    	static Rect& obtain(T cont) noexcept {
			return std::invoke(trans, cont);
		}

    	[[nodiscard]] bool contains(T cont) const noexcept {return all.contains(cont);}

    	std::vector<T> boxes_widthAscend{};
    	std::vector<T> boxes_heightAscend{};
    	std::unordered_set<T> all{};

    	using SizeTy = Geom::Vector2D<N>;
    public:
    	std::vector<T> packed{};

    	SizeTy exportSize{};
    	SizeTy maxSize{};

    	N margin{0};

    	constexpr void setMaxSize(const SizeTy size) noexcept {
    		maxSize.set(size);
    	}

    	constexpr void setMaxSize(const N maxWidth, const N maxHeight) noexcept {
    		maxSize.set(maxWidth, maxHeight);
    	}

		[[nodiscard]] StripPacker2D() = default;

    	template <Concepts::Iterable<T> Rng>
			requires std::ranges::sized_range<Rng>
		[[nodiscard]] explicit StripPacker2D(Rng& targets){
    		this->template push<Rng>(targets);
    	}

    	template <Concepts::Iterable<T> Rng>
    		requires std::ranges::sized_range<Rng>
    	void push(Rng& targets){
    		all.reserve(targets.size());
    		boxes_widthAscend.reserve(targets.size());
    		boxes_heightAscend.reserve(targets.size());

    		for(auto& element : targets) { //Why transform crashes?
    			all.insert(element);
    			boxes_widthAscend.push_back(element);
    			boxes_heightAscend.push_back(element);
    		}

    		packed.reserve(all.size());
    	}

		void sortData() {
    		std::ranges::sort(boxes_widthAscend , std::greater<N>{}, [](const T& t){
    			return StripPacker2D::obtain(t).getWidth();
    		});

    		std::ranges::sort(boxes_heightAscend, std::greater<N>{}, [](const T& t){
				return StripPacker2D::obtain(t).getHeight();
			});
    	}

		[[nodiscard]] constexpr N getMargin() const noexcept{ return margin; }

		constexpr void setMargin(const N margin) noexcept{ this->margin = margin; }

		void process() noexcept {
    		sortData();

    		if(margin != 0){
    			for (auto& data : boxes_widthAscend){
    				this->obtain(data).addSize(margin * 2, margin * 2);
    			}
    		}
    		this->tryPlace({SubRectArr{Rect{}, Rect{}, Rect{maxSize}}});
    		if(margin != 0){
    			for (auto& data : boxes_widthAscend){
    				this->obtain(data).shrink(margin, margin);
    			}
    		}
    	}

    	Geom::Vector2D<N> getResultSize() const noexcept {return exportSize;}

    	std::unordered_set<T>& getRemains() noexcept {return all;}

    protected:
    	[[nodiscard]] bool shouldStop() const noexcept {return all.empty();}

    	Rect* tryPlace(const Rect& bound, std::vector<T>& targets) noexcept {
    		for(auto& cont : targets){
    			if(!this->contains(cont))continue;

    			Rect& rect = this->obtain(cont);

    			rect.setSrc(bound.getSrcX(), bound.getSrcY());

    			if( //Contains
					rect.getEndX() <= bound.getEndX() &&
					rect.getEndY() <= bound.getEndY()
				){

    				all.erase(cont);
    				packed.push_back(cont);

    				exportSize.maxX(rect.getEndX());
    				exportSize.maxY(rect.getEndY());

    				return &rect;
				}

    		}

    		return nullptr;
    	}


	    /**
    	 * \brief 
    	 * @code                                                                                             
    	 *              |                                      
    	 *    array[0]  |  array[2]                            
    	 *              |                                
    	 * -------------|---------------
    	 *              |                      
    	 *      Box     |  array[1]                            
    	 *              |                                                                        
    	 * @endcode 
    	 */
    	constexpr SubRectArr splitQuad(const Rect& bound, const Rect& box) {
#if DEBUG_CHECK
    		if(bound.getSrcX() != box.getSrcX() || box.getSrcY() != box.getSrcY())throw ext::IllegalArguments{"The source of the box and the bound doesn't match"};
#endif
    		return SubRectArr{
    			Rect{bound.getSrcX(),   box.getEndY(),   box.getWidth()                 , bound.getHeight() - box.getHeight()},
    			Rect{  box.getEndX(), bound.getSrcY(), bound.getWidth() - box.getWidth(),   box.getHeight()                  },
    			Rect{  box.getEndX(),   box.getEndY(), bound.getWidth() - box.getWidth(), bound.getHeight() - box.getHeight()}
    		};
    	}

    	void tryPlace(std::vector<SubRectArr>&& bounds) noexcept {
			if(shouldStop())return;

    		std::vector<SubRectArr> next{};
    		next.reserve(bounds.size() * 3);

    		//BFS
			for(const SubRectArr& currentBound : bounds) {
				for(const auto& [i, bound] : currentBound | std::ranges::views::enumerate){
					if(bound.area() == 0)continue;

					if(const Rect* const result = this->tryPlace(bound, (i & 1) ? boxes_widthAscend : boxes_heightAscend)){
						//if(result != nullptr)
						auto arr = this->splitQuad(bound, *result);
						if(arr[0].area() == 0 && arr[1].area() == 0 && arr[2].area() == 0)continue;
						next.push_back(std::move(arr));
					}
				}
			}

    		if(!next.empty())this->tryPlace(std::move(next));
    	}
    };
}
