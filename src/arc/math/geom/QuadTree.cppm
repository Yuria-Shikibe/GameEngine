// ReSharper disable CppDFAUnreachableCode
module;

export module Geom.QuadTree;

export import Geom.Rect_Orthogonal;
export import Geom.Vector2D;

import ext.Concepts;
import ext.RuntimeException;
import std;

//TODO The design is so bad!
export namespace Geom{
	template <typename Cont, Concepts::Number T, Concepts::InvokeNullable<Geom::Rect_Orthogonal<T>(const Cont&)> auto
	          transformer = nullptr>
	class QuadTree{
		using Rect = Geom::Rect_Orthogonal<T>;

	public:
		using NumType = T;
		using ValueType = Cont;
		using SubTree = std::array<QuadTree*, 4>;
		// using Obtainer = std::function<const Rect&(const Cont*)>;
		using InersectCheck = std::function<bool(const Cont*, const Cont*)>;

		using PointType = Geom::Vector2D<T>;
		using InersectPointCheck = std::function<bool(const Cont*, Vec2)>;

		static constexpr bool ValidTrans = !std::same_as<std::nullptr_t, decltype(transformer)>;

	protected:
		// Obtainer transformer{ nullptr };
		InersectCheck interscetExactFunc{nullptr};
		InersectCheck interscetRoughFunc{nullptr};

		InersectPointCheck interscetPointFunc{nullptr};
		// The boundary of this node
		Rect boundary{};

		std::mutex containerlock{};
		std::mutex subTreelock{};

		bool isInersectedBetween(const Cont* subject, const Cont* object){
			if(subject == object) return false;
			const bool intersected =
				this->obtainBound(subject).overlap(this->obtainBound(object))
				&& (!interscetRoughFunc || interscetRoughFunc(subject, object))
				&& (!interscetExactFunc || interscetExactFunc(subject, object));

			return intersected;
		}

		Rect obtainBound(const Cont* const cont){
			if constexpr(ValidTrans) return transformer(*cont);
			else return boundary;
		}

		Rect obtainBound(const Cont& cont){
			if constexpr(ValidTrans) return transformer(cont);
			else return boundary;
		}

		// The rectangles in this node
		std::vector<Cont*> rectangles{};

		// The four children of this node
		std::unique_ptr<QuadTree> topLeft{nullptr};
		std::unique_ptr<QuadTree> topRight{nullptr};
		std::unique_ptr<QuadTree> bottomLeft{nullptr};
		std::unique_ptr<QuadTree> bottomRight{nullptr};

		unsigned int maximumItemCount = 4;
		bool strict = false;

		std::atomic_uint currentSize = 0;
		std::atomic_bool leaf = true;

	public:
		QuadTree(const QuadTree& other) = delete;

		QuadTree(QuadTree&& other) noexcept
			: interscetExactFunc(std::move(other.interscetExactFunc)),
			  interscetRoughFunc(std::move(other.interscetRoughFunc)),
			  interscetPointFunc(std::move(other.interscetPointFunc)),
			  boundary(std::move(other.boundary)),
			  rectangles(std::move(other.rectangles)),
			  topLeft(std::move(other.topLeft)),
			  topRight(std::move(other.topRight)),
			  bottomLeft(std::move(other.bottomLeft)),
			  bottomRight(std::move(other.bottomRight)),
			  maximumItemCount(other.maximumItemCount),
			  strict(other.strict),
			  currentSize(other.currentSize.load()),
			  leaf(other.leaf.load()){}

		QuadTree& operator=(const QuadTree& other) = delete;

		QuadTree& operator=(QuadTree&& other) noexcept{
			if(this == &other) return *this;
			interscetExactFunc = std::move(other.interscetExactFunc);
			interscetRoughFunc = std::move(other.interscetRoughFunc);
			interscetPointFunc = std::move(other.interscetPointFunc);
			boundary = std::move(other.boundary);
			rectangles = std::move(other.rectangles);
			topLeft = std::move(other.topLeft);
			topRight = std::move(other.topRight);
			bottomLeft = std::move(other.bottomLeft);
			bottomRight = std::move(other.bottomRight);
			maximumItemCount = other.maximumItemCount;
			strict = other.strict;
			currentSize = other.currentSize.load();
			leaf = other.leaf.load();
			return *this;
		}

		[[nodiscard]] bool isStrict() const{ return strict; }

		void setStrict(const bool strict){ this->strict = strict; }

		[[nodiscard]] unsigned int size() const{
			return currentSize;
		}

		void setExactInterscet(const InersectCheck& interscetExactJudger){
			this->interscetExactFunc = interscetExactJudger;
		}

		void setRoughInterscet(const InersectCheck& interscetRoughJudger){
			this->interscetRoughFunc = interscetRoughJudger;
		}

		void setPointInterscet(const InersectPointCheck& interscetPointJudger){
			this->interscetPointFunc = interscetPointJudger;
		}

		explicit QuadTree(unsigned int maxCount){
			rectangles.reserve(maxCount);
		}

		explicit QuadTree(const Rect& boundary) : boundary(boundary){
			rectangles.reserve(maximumItemCount);
		}

		QuadTree(const Rect& boundary, unsigned int maxCount,
		         const InersectCheck& interectRoughJudger,
		         const InersectCheck& interectExactJudger,
		         const InersectPointCheck& interectPointJudger
		) :
			interscetExactFunc(interectExactJudger),
			interscetRoughFunc(interectRoughJudger),
			interscetPointFunc(interectPointJudger),
			boundary(boundary), maximumItemCount(maxCount){
			this->rectangles.reserve(maxCount);
		}

		[[nodiscard]] Rect& getBoundary(){
			return boundary;
		}

		void setBoundary(const Rect& boundary){
			if(!allChildrenEmpty()) throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

			this->boundary = boundary;
		}

		void setBoundary(const T x, const T y, const T width, const T height){
			this->boundary.set(x, y, width, height);
		}

		void setBoundary(const T width, const T height){
			if(!allChildrenEmpty()) throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

			this->boundary.setSize(width, height);
		}

		bool remove(Cont* box){
			bool result;
			if(isLeaf()){
				std::lock_guard guard{containerlock};
				result = static_cast<bool>(std::erase(rectangles, box));
			} else{
				if(QuadTree* child = this->getFittingChild(this->obtainBound(box))){
					result = child->remove(box);
				} else{
					std::lock_guard guard{containerlock};
					result = static_cast<bool>(std::erase(rectangles, box));
				}

				if(currentSize <= maximumItemCount) unsplit();
			}

			if(result){
				--currentSize;
				if(allChildrenEmpty()){
					leaf = true;
				}
			}

			return result;
		}

		[[nodiscard]] bool isLeaf() const{
			return leaf;
		}

		[[nodiscard]] bool allChildrenEmpty() const{
			return currentSize == rectangles.size();
		}

		template <Concepts::Invokable<void(QuadTree*)> Func>
		void each(Func&& func){
			func(this);

			if(!isLeaf()){
				topLeft->each(std::forward<Func>(func));
				topRight->each(std::forward<Func>(func));
				bottomLeft->each(std::forward<Func>(func));
				bottomRight->each(std::forward<Func>(func));
			}
		}

		bool intersectAny(Cont* object){
			if(!this->inbound(object)){
				return false;
			}

			if(std::ranges::any_of(rectangles, [this, object](const Cont* cont){
				return this->isInersectedBetween(object, cont);
			})){
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				//TODO uses direction vector to get more possible results?
				return
					topLeft->intersectAny(object) ||
					topRight->intersectAny(object) ||
					bottomLeft->intersectAny(object) ||
					bottomRight->intersectAny(object);
			}

			// Otherwise, the rectangle does not overlap with any rectangle in the quad tree
			return false;
		}

		bool intersectAny(const Vec2 point){
			if(!this->inbound(point)){
				return false;
			}

			if(std::ranges::any_of(rectangles, [this, point](const Cont* cont){
				return this->intersectWith(cont, point);
			})){
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				//TODO uses direction vector to get more possible results?
				return
					topLeft->intersectAny(point) ||
					topRight->intersectAny(point) ||
					bottomLeft->intersectAny(point) ||
					bottomRight->intersectAny(point);
			}

			// Otherwise, the rectangle does not overlap with any rectangle in the quad tree
			return false;
		}

		void intersectAll(const Cont* object){
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				this->topLeft->intersectAll(object);
				this->topRight->intersectAll(object);
				this->bottomLeft->intersectAll(object);
				this->bottomRight->intersectAll(object);
			}

			for(auto& element : rectangles){
				this->isInersectedBetween(object, element);
			}
		}

		template <Concepts::Invokable<void(Cont*)> Pred>
		void intersect(const Cont* object, Pred&& pred){
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				topLeft->intersect(object, pred);
				topRight->intersect(object, pred);
				bottomLeft->intersect(object, pred);
				bottomRight->intersect(object, pred);
			}

			for(const auto cont : rectangles){
				if(this->isInersectedBetween(object, cont)){
					pred(cont);
				}
			}
		}

		template <Concepts::Invokable<void(Cont*, const Rect&)> Func>
		void intersectRect(const Rect& rect,  Func&& pred){
			if(!this->inbound(rect)) return;

			//If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				topLeft->intersectRect(rect, pred);
				topRight->intersectRect(rect, pred);
				bottomLeft->intersectRect(rect, pred);
				bottomRight->intersectRect(rect, pred);
			}

			for(auto cont : rectangles){
				pred(cont, rect);
			}
		}

		template <typename Region, Concepts::Invokable<bool(const Rect&, const Region&)> Check, Concepts::Invokable<void(Cont*, const Region&)> Pred>
			requires !std::same_as<Region, Rect>
		void intersectRegion(const Region& region, Check&& boundCheck,  Pred&& pred){
			if(!boundCheck(boundary, region)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				topLeft->template intersectRegion<Region>(region, boundCheck, pred);
				topRight->template intersectRegion<Region>(region, boundCheck, pred);
				bottomLeft->template intersectRegion<Region>(region, boundCheck, pred);
				bottomRight->template intersectRegion<Region>(region, boundCheck, pred);
			}

			for(const auto cont : rectangles){
				if(boundCheck(this->obtainBound(cont), region))pred(cont, region);
			}
		}

		template <Concepts::Invokable<void(Cont*)> Pred>
		void intersectPoint(const Vec2 point, Pred&& pred){
			if(!this->inbound(point)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				topLeft->intersectPoint(point, std::forward<Pred>(pred));
				topRight->intersectPoint(point, std::forward<Pred>(pred));
				bottomLeft->intersectPoint(point, std::forward<Pred>(pred));
				bottomRight->intersectPoint(point, std::forward<Pred>(pred));
			}

			for(const auto cont : rectangles){
				if(this->intersectWith(cont, point)){
					pred(cont);
				}
			}
		}

		template <Concepts::Invokable<void(Cont*)> Pred>
		void within(const Cont* object, const T dst, Pred&& pred){
			if(!this->withinBound(object, dst)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				topLeft->within(object, dst, pred);
				topRight->within(object, dst, pred);
				bottomLeft->within(object, dst, pred);
				bottomRight->within(object, dst, pred);
			}

			for(const auto& cont : rectangles){
				pred(cont);
			}
		}

		template <Concepts::Invokable<void(Cont*)> Pred>
		bool intersectOnce(const Cont* object, Pred&& pred){
			if(!this->inbound(object)) return false;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()){
				return topLeft->intersect(object, pred) ||
					topRight->intersect(object, pred) ||
					bottomLeft->intersect(object, pred) ||
					bottomRight->intersect(object, pred);
			}

			bool intersected = false;

			for(const auto& cont : rectangles){
				if(this->isInersectedBetween(object, cont)){
					intersected = true;
					pred(cont);
					break;
				}
			}

			return intersected;
		}

		bool intersectWith(const Cont* object, const Vec2 point){
			return this->obtainBound(object).containsPos_edgeExclusive(point) && (!static_cast<bool>(interscetPointFunc)
				|| interscetPointFunc(object, point));
		}

		bool inbound(const Rect& rect){
			if(strict) return boundary.contains(rect);
			return boundary.overlap(rect);
		}

		bool inbound(const Cont* object){
			if(strict) return boundary.contains(this->obtainBound(object));
			return boundary.overlap(this->obtainBound(object));
		}

		bool inbound(const Vec2 object){
			return boundary.containsPos_edgeExclusive(object);
		}

		bool withinBound(const Cont* object, const T dst){
			return this->obtainBound(object).getCenter().within(boundary.getCenter(), dst);
		}

		bool insert(Cont* box){
			// Ignore objects that do not belong in this quad tree
			if(!this->inbound(box)){
				return false;
			}

			//If this box is inbound, it should must be added.
			++currentSize;

			// Otherwise, subdivide and then add the object to whichever node will accept it
			if(isLeaf()){
				if(rectangles.size() >= maximumItemCount){
					split();
				} else{
					std::lock_guard guard{containerlock};
					rectangles.push_back(box);
					return true;
				}
			}

			const Rect& rect = this->obtainBound(box);
			// Add to relevant child, or root if can't fit completely in a child
			if(QuadTree* child = this->getFittingChild(rect)){
				return child->insert(box);
			}

			//Fallback
			std::lock_guard guard{containerlock};
			rectangles.push_back(box);
			return true;
		}

		void clear(){
			topLeft.reset(nullptr);
			topRight.reset(nullptr);
			bottomLeft.reset(nullptr);
			bottomRight.reset(nullptr);

			currentSize = 0;

			leaf = true;
			rectangles.clear();
		}

		void clearItemsOnly(){
			if(!isLeaf()){
				topLeft->clearItemsOnly();
				topRight->clearItemsOnly();
				bottomLeft->clearItemsOnly();
				bottomRight->clearItemsOnly();
			}

			leaf = true;
			currentSize = 0;
			rectangles.clear();
		}

		void split(){
			if(!isLeaf()) return;
			leaf = false;
			if(topLeft != nullptr) return;

			const T x = boundary.getSrcX();
			const T y = boundary.getSrcY();
			const T w = boundary.getWidth() / static_cast<T>(2);
			const T h = boundary.getHeight() / static_cast<T>(2);

			const Rect bl{x, y, w, h};
			const Rect br{x + w, y, w, h};
			const Rect tl{x, y + h, w, h};
			const Rect tr{x + w, y + h, w, h};

			if(std::lock_guard guard{subTreelock}; topLeft != nullptr){
				topLeft->setBoundary(tl);
				topRight->setBoundary(tr);
				bottomLeft->setBoundary(bl);
				bottomRight->setBoundary(br);
			}
			{
				topLeft = std::make_unique<QuadTree>(tl, maximumItemCount, interscetRoughFunc, interscetExactFunc,
				                                     interscetPointFunc);
				topRight = std::make_unique<QuadTree>(tr, maximumItemCount, interscetRoughFunc, interscetExactFunc,
				                                      interscetPointFunc);
				bottomLeft = std::make_unique<QuadTree>(bl, maximumItemCount, interscetRoughFunc, interscetExactFunc,
				                                        interscetPointFunc);
				bottomRight = std::make_unique<QuadTree>(br, maximumItemCount, interscetRoughFunc, interscetExactFunc,
				                                         interscetPointFunc);
			}
		}

		void unsplit(){
			if(isLeaf()) return;
			leaf = true;

			{
				std::scoped_lock guard{containerlock};
				std::ranges::copy(topLeft->rectangles, std::back_inserter(rectangles));
				std::ranges::copy(topRight->rectangles, std::back_inserter(rectangles));
				std::ranges::copy(bottomLeft->rectangles, std::back_inserter(rectangles));
				std::ranges::copy(bottomRight->rectangles, std::back_inserter(rectangles));
			}

			topLeft->clearItemsOnly();
			topRight->clearItemsOnly();
			bottomLeft->clearItemsOnly();
			bottomRight->clearItemsOnly();
		}

		QuadTree* getFittingChild(const Rect boundingBox){
			if(isLeaf()) return nullptr;
			const T verticalMidpoint = boundary.getSrcX() + boundary.getWidth() / 2;
			const T horizontalMidpoint = boundary.getSrcY() + boundary.getHeight() / 2;

			// Object can completely fit within the top quadrants
			const bool topQuadrant = boundingBox.getSrcY() > horizontalMidpoint;
			// Object can completely fit within the bottom quadrants
			const bool bottomQuadrant = boundingBox.getSrcY() < horizontalMidpoint && (
				boundingBox.getSrcY() + boundingBox.getHeight()) < horizontalMidpoint;

			// Object can completely fit within the left quadrants
			if(boundingBox.getSrcX() < verticalMidpoint && boundingBox.getSrcX() + boundingBox.getWidth() <
				verticalMidpoint){
				if(topQuadrant){
					return topLeft.get();
				} else if(bottomQuadrant){
					return bottomLeft.get();
				}
			} else if(boundingBox.getSrcX() > verticalMidpoint){
				// Object can completely fit within the right quadrants
				if(topQuadrant){
					return topRight.get();
				} else if(bottomQuadrant){
					return bottomRight.get();
				}
			}

			return nullptr;
		}
	};


	template <typename T, auto func>
	using QuadTreeF = QuadTree<T, float, func>;
}
