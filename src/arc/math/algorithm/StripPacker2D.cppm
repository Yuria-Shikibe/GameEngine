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
    template <typename Cont, typename T>
		requires Concepts::Number<T> && std::is_integral_v<T>
	struct Packer {
		using Rect = Geom::Shape::Rect_Orthogonal<T>;
    	using obtainer = std::function<Rect&(Cont&)>;
    	using subRectArr = std::array<Rect, 3>;

    	obtainer trans{nullptr};

    	std::vector<Cont*> boxes_widthAscend{};
    	std::vector<Cont*> boxes_heightAscend{};

    	std::unordered_set<Cont*> all{};

    	std::vector<Cont*> packed{};

    	T currentWidth{0}, currentHeight{0};
    	T width{0}, height{0};
    	T maxWidth {2750};
    	T maxHeight{2750};

    	Rect& obtain(Cont& cont) {
    		return trans(cont);
    	}

    	//TODO MERGE THESE
    	Packer(const std::unordered_set<Cont>& targets, const obtainer& trans) : trans(trans)
    	{
    		all.reserve(targets.size());
    		boxes_widthAscend.reserve(targets.size());
    		boxes_heightAscend.reserve(targets.size());
		    for(auto& element : targets) { //Why transform crashes?
			    all.insert(&element);
			    boxes_widthAscend.push_back(&element);
			    boxes_heightAscend.push_back(&element);
		    }
    	}

    	Packer(const std::vector<Cont*>& targets, const obtainer& trans) : trans(trans)
    	{
    		all.reserve(targets.size());
    		boxes_widthAscend.reserve(targets.size());
    		boxes_heightAscend.reserve(targets.size());
    		for(auto element : targets) { //Why transform crashes?
    			all.insert(element);
    			boxes_widthAscend.push_back(element);
    			boxes_heightAscend.push_back(element);
    		}
    	}

    	Packer(const std::unordered_set<Cont*>& targets, const obtainer& trans) : trans(trans)
    	{
    		all.reserve(targets.size());
    		boxes_widthAscend.reserve(targets.size());
    		boxes_heightAscend.reserve(targets.size());
    		for(auto element : targets) { //Why transform crashes?
    			all.insert(element);
    			boxes_widthAscend.push_back(element);
    			boxes_heightAscend.push_back(element);
    		}
    	}

    	Packer(auto begin, auto end, const obtainer& trans) : trans(trans)
    	{
    		all.reserve(end - begin);
    		boxes_widthAscend.reserve(end - begin);
    		boxes_heightAscend.reserve(end - begin);
    		for(auto t = begin; t != end; ++t) {
    			all.insert(&(*t));
    			boxes_widthAscend.push_back(&(*t));
    			boxes_heightAscend.push_back(&(*t));
    		}
    	}

    	void begin() {
    		std::ranges::sort(boxes_widthAscend, [this](Cont* r1, Cont* r2) {
    			return obtain(*r1).getWidth() > obtain(*r2).getWidth();
    		});

    		std::ranges::sort(boxes_heightAscend, [this](Cont* r1, Cont* r2) {
				return obtain(*r1).getHeight() > obtain(*r2).getHeight();
			});
    	}

    	Rect& getLargest(std::vector<Cont*>& which) {
    		Cont* back = which.back();
    		Rect& rect = obtain(back);
    		all.erase(back);

    		return rect;
    	}

    	[[nodiscard]] bool contains(Cont* const cont) const {
    		return all.contains(cont);
    	}


    	/**
		 * \brief The Source Of The Params Must Be Aligned
		 * \param bound
		 * \param box
		 * \return
		 */
    	// ReSharper disable once CppMemberFunctionMayBeStatic
    	bool canPlace(const Rect& bound, const Rect& box) const {
    		return box.getEndX() < bound.getEndX() && box.getEndY() < bound.getEndY();
    	}

    	bool shouldStop() {
    		return all.empty();
    	}

    	Rect* tryPlace(const Rect& bound, std::vector<Cont*>& which) {

    		for(auto itr = which.begin(); itr != which.end(); ++itr){
    			if(!contains(*itr))continue;
    			Rect& rect = obtain(**itr);
    			rect.setSrc(bound.getSrcX(), bound.getSrcY());
    			if(canPlace(bound, rect)) {
    				all.erase(*itr);
    				packed.push_back(*itr);
    				currentWidth = std::max(rect.getEndX(), currentWidth);
    				currentHeight = std::max(rect.getEndY(), currentHeight);
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
    		if(bound.getSrcX() != box.getSrcX() || box.getSrcY() != box.getSrcY()) {
    			throw ext::IllegalArguments{"The source of the box and the bound doesn't match"};
    		}

    		return subRectArr{
    			Rect{bound.getSrcX(), box.getEndY(), box.getWidth(), bound.getHeight() - box.getHeight()},
    			Rect{box.getEndX(), bound.getSrcY(), bound.getWidth() - box.getWidth(), box.getHeight()},
    			Rect{box.getEndX(), box.getEndY(), bound.getWidth() - box.getWidth(), bound.getHeight() - box.getHeight()}
    		};
    	}

    	void tryPlace(subRectArr&& currentBound) {

			if(shouldStop())return;

    		const Rect& bound_Left     = currentBound[0];
    		const Rect& bound_Bottom   = currentBound[1];
    		const Rect& bound_Diagonal = currentBound[2];

    		if(bound_Left.area() > 0) {
    			if(const Rect* const result = tryPlace(bound_Left, boxes_heightAscend)){
    				//if(result != nullptr)
    				tryPlace(splitQuad(bound_Left, *result));
    			}
    		}

    		if(bound_Bottom.area() > 0) {
    			if(const Rect* const result = tryPlace(bound_Bottom, boxes_widthAscend)){
    				//if(result != nullptr)
    				tryPlace(splitQuad(bound_Bottom, *result));
    			}
    		}

    		if(bound_Diagonal.area() > 0) {
    			if(const Rect* const result = tryPlace(bound_Diagonal, boxes_heightAscend)){
    				//if(result != nullptr)
    				tryPlace(splitQuad(bound_Diagonal, *result));
    			}
    		}
    	}

    	Rect process() {
    		tryPlace(std::array<Rect, 3>{Rect{}, Rect{}, Rect{maxWidth, maxHeight}});

    		return Rect{0, 0, currentWidth, currentHeight};
    	}

    	std::unordered_set<Cont*>& remains() {
    		return all;
    	}
    };
}
