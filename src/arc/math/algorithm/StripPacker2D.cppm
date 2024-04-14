module;

export module Math.StripPacker2D;

import std;
import ext.RuntimeException;
import Concepts;
import Geom.Rect_Orthogonal;
import Math;

export namespace Math {
	/**
     * @brief
     * @tparam Tgt Should Avoid Copy When Tgt Type is provided
     * @tparam N pack arithmetic type
     * @tparam trans Trans the Tgt to a non-temp rect
     */
    template <typename Tgt, Concepts::Number N, auto trans>
		requires Concepts::Invokable<decltype(trans), Geom::Rect_Orthogonal<N>&(Tgt)> && Concepts::InvokeNoexcept<decltype(trans)>
	struct StripPacker2D {
    protected:
		using Rect = Geom::Rect_Orthogonal<N>;
    	using SubRectArr = std::array<Rect, 3>;

    	static Rect& obtain(Tgt cont) noexcept {
			return trans(cont);
		}

    	[[nodiscard]] bool contains(Tgt cont) const noexcept {return all.contains(cont);}

    	std::vector<Tgt> boxes_widthAscend{};
    	std::vector<Tgt> boxes_heightAscend{};
    	std::unordered_set<Tgt> all{};

    public:
    	std::vector<Tgt> packed{};

    	N rstWidth{0}, rstHeight{0};
    	N maxWidth{2750}, maxHeight{2750};

    	void setMaxSize(const N maxWidth, const N maxHeight) noexcept {
    		this->maxHeight = maxHeight;
    		this->maxWidth = maxWidth;
    	}

		[[nodiscard]] StripPacker2D() = default;

    	void push(Concepts::Iterable<Tgt> auto& targets){
    		all.reserve(targets.size());
    		boxes_widthAscend.reserve(targets.size());
    		boxes_heightAscend.reserve(targets.size());
    		for(auto element : targets) { //Why transform crashes?
    			all.insert(element);
    			boxes_widthAscend.push_back(element);
    			boxes_heightAscend.push_back(element);
    		}

    		packed.reserve(all.size());
    	}

		void sortDatas() {
    		std::ranges::sort(boxes_widthAscend , [this](const Rect& r1, const Rect& r2) {
    			return r1.getWidth() > r2.getWidth();
    		}, &obtain);

    		std::ranges::sort(boxes_heightAscend, [this](const Rect& r1, const Rect& r2) {
				return r1.getHeight() > r2.getHeight();
			}, &obtain);
    	}

    	void process() noexcept {
    		this->tryPlace({SubRectArr{Rect{}, Rect{}, Rect{maxWidth, maxHeight}}});
    	}

    	Rect getResultBound() const noexcept {return Rect{0, 0, rstWidth, rstHeight};}

    	std::unordered_set<Tgt>& getRemains() noexcept {return all;}

    protected:
    	[[nodiscard]] bool shouldStop() const noexcept {return all.empty();}

    	Rect* tryPlace(const Rect& bound, std::vector<Tgt>& targets) noexcept {
    		for(auto& cont : targets){
    			if(!this->contains(cont))continue;
    			Rect& rect = this->obtain(cont);
    			rect.setSrc(bound.getSrcX(), bound.getSrcY());
    			if(
					rect.getEndX() <= bound.getEndX() &&
					rect.getEndY() <= bound.getEndY()
				){

    				all.erase(cont);
    				packed.push_back(cont);
    				rstWidth = Math::max(rect.getEndX(), rstWidth);
    				rstHeight = Math::max(rect.getEndY(), rstHeight);

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
#ifdef _DEBUG
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

			for(const SubRectArr& currentBound : bounds) {
				for(int i = 0; i < currentBound.size(); ++i) {
					if(const Rect& bound = currentBound[i]; bound.area() > 0) {
						if(const Rect* const result = this->tryPlace(bound, (i & 1) ? boxes_widthAscend : boxes_heightAscend)){
							//if(result != nullptr)
							auto arr = this->splitQuad(bound, *result);
							if(arr[0].area() == 0 && arr[1].area() == 0 && arr[2].area() == 0)continue;
							next.push_back(std::move(arr));
						}
					}
				}
			}

    		if(!next.empty())this->tryPlace(std::move(next));
    	}
    };
}
