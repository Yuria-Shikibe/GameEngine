module;

export module Math.StripPacker2D;

import <vector>;
import <ranges>;
import <span>;
import <algorithm>;
import RuntimeException;
import Concepts;
import Geom.QuadTree;
export import Geom.Shape.Rect_Orthogonal;
import <functional>;
import <numeric>;
import <unordered_set>;

export namespace Math {
    template <typename Cont, Concepts::Number T>
	struct StripPacker2D {
    protected:
		using Rect = Geom::Shape::Rect_Orthogonal<T>;
    	using obtainer = std::function<Rect&(Cont&)>;
    	using subRectArr = std::array<Rect, 3>;

    	const obtainer& trans;

    	Rect& obtain(Cont& cont) {
    		return trans(cont);
    	}

    	[[nodiscard]] bool contains(Cont* const cont) const {return all.contains(cont);}

    	std::vector<Cont*> boxes_widthAscend{};
    	std::vector<Cont*> boxes_heightAscend{};
    	std::unordered_set<Cont*> all{};

    public:
    	std::vector<Cont*> packed{};

    	T rstWidth{0}, rstHeight{0};
    	T maxWidth{2750}, maxHeight{2750};

    	void setMaxSize(const T maxWidth, const T maxHeight) {
    		this->maxHeight = maxHeight;
    		this->maxWidth = maxWidth;
    	}

		[[nodiscard]] explicit StripPacker2D(const obtainer& trans)
    		: trans(trans) {
#ifdef DEBUG_LOCAL
    		if(!trans)throw ext::RuntimeException{"Empty Obtainer!!"};
#endif
    	}

    	template <Concepts::Iterable<Cont*> Range>
    		requires requires (Range range){std::is_same_v<size_t, decltype(range.size())>;}
		void push(const Range& targets){
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
    		std::ranges::sort(boxes_widthAscend , [this](Cont* r1, Cont* r2) {
    			return obtain(*r1).getWidth() > obtain(*r2).getWidth();
    		});

    		std::ranges::sort(boxes_heightAscend, [this](Cont* r1, Cont* r2) {
				return obtain(*r1).getHeight() > obtain(*r2).getHeight();
			});
    	}

    	void process() {
    		tryPlace({subRectArr{Rect{}, Rect{}, Rect{maxWidth, maxHeight}}});
    	}

    	Rect resultBound() const {return Rect{0, 0, rstWidth, rstHeight};}

    	std::unordered_set<Cont*>& remains() {return all;}

    protected:
    	bool shouldStop() const {return all.empty();}

    	Rect* tryPlace(const Rect& bound, std::vector<Cont*>& which) {
    		for(auto itr = which.begin(); itr != which.end(); ++itr){
    			if(!this->contains(*itr))continue;
    			Rect& rect = obtain(**itr);
    			rect.setSrc(bound.getSrcX(), bound.getSrcY());
    			if(
    				rect.getEndX() <= bound.getEndX() &&
    				rect.getEndY() <= bound.getEndY()
    			){

    				all.erase(*itr);
    				packed.push_back(*itr);
    				rstWidth = std::max(rect.getEndX(), rstWidth);
    				rstHeight = std::max(rect.getEndY(), rstHeight);

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
    	subRectArr splitQuad(const Rect& bound, const Rect& box) {
#ifdef DEBUG_LOCAL
    		if(bound.getSrcX() != box.getSrcX() || box.getSrcY() != box.getSrcY())throw ext::IllegalArguments{"The source of the box and the bound doesn't match"};
#endif
    		return subRectArr{
    			Rect{bound.getSrcX(),   box.getEndY(),   box.getWidth()                 , bound.getHeight() - box.getHeight()},
    			Rect{  box.getEndX(), bound.getSrcY(), bound.getWidth() - box.getWidth(),   box.getHeight()                  },
    			Rect{  box.getEndX(),   box.getEndY(), bound.getWidth() - box.getWidth(), bound.getHeight() - box.getHeight()}
    		};
    	}

    	void tryPlace(std::vector<subRectArr>&& bounds) {
			if(shouldStop())return;

    		std::vector<subRectArr> next{};
    		next.reserve(bounds.size() * 3);

			for(const subRectArr& currentBound : bounds) {
				for(int i = 0; i < currentBound.size(); ++i) {
					if(const Rect& bound = currentBound[i]; bound.area() > 0) {
						if(const Rect* const result = tryPlace(bound, (i & 1) ? boxes_widthAscend : boxes_heightAscend)){
							//if(result != nullptr)
							auto arr = splitQuad(bound, *result);
							if(arr[0].area() == 0 && arr[1].area() == 0 && arr[2].area() == 0)continue;
							next.push_back(std::move(arr));
						}
					}
				}
			}

    		if(!next.empty())tryPlace(std::move(next));
    	}
    };
}
