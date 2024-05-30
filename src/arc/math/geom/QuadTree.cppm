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

	// template <typename T>
	// struct AddressEqual{
	// 	bool operator()(const std::reference_wrapper<T>& l, const std::reference_wrapper<T>& r) const noexcept{
	// 		return &l.get() = &r.get();
	// 	}
	// };

	// using ItemTy = int;
	// using T = float;
	// constexpr auto transformer = std::identity{};
	// using Equal = AddressEqual<ItemTy>;

	template <typename ItemTy, Concepts::Number T, Concepts::InvokeNullable<Rect_Orthogonal<T>(const ItemTy&)> auto
	          transformer = nullptr>
	class QuadTree{
	public:
		using Rect = Geom::Rect_Orthogonal<T>;
		using ArithmeticType = T;
		using ValueType = ItemTy;

		using PointType = Geom::Vector2D<ArithmeticType>;

		// using Obtainer = std::function<const Rect&(const Cont*)>;
		using InersectCheckFunc = std::function<bool(const ItemTy&, const ItemTy&)>;
		using InersectPointCheckFunc = std::function<bool(const ItemTy&, Vec2)>;

		static constexpr bool ValidTrans = !std::same_as<std::nullptr_t, decltype(transformer)>;

	protected:
		// Obtainer transformer{ nullptr };
		// TODO apply in func params??
		InersectCheckFunc interscetExactFunc{nullptr};
		InersectCheckFunc interscetRoughFunc{nullptr};

		InersectPointCheckFunc interscetPointFunc{nullptr};

		// The boundary of this node
		Rect boundary{};

		// std::mutex containerlock{};
		// std::mutex subTreelock{};

		mutable std::shared_mutex itemsMtx{};
		mutable std::shared_mutex leafMtx{};

		void setBoundary_enforce(const Rect& boundary){
			if(!allChildrenEmpty()) throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

			this->boundary = boundary;
		}

		bool isInersectedBetween(const ItemTy& subject, const ItemTy& object){
			//TODO equalTy support?
			if(&subject == &object) return false;
			const bool intersected =
				this->obtainBound(subject).overlap(this->obtainBound(object))
				&& (!interscetRoughFunc || std::invoke(interscetRoughFunc, subject, object))
				&& (!interscetExactFunc || std::invoke(interscetExactFunc, subject, object));

			return intersected;
		}

		Rect obtainBound(const ItemTy& cont) const{
			if constexpr(ValidTrans) return transformer(cont);
			else return boundary;
		}

		// The rectangles in this node
		std::vector<std::reference_wrapper<ItemTy>> items{};

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
			  items(std::move(other.items)),
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
			items = std::move(other.items);
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

		void setExactInterscet(const InersectCheckFunc& interscetExactJudger){
			this->interscetExactFunc = interscetExactJudger;
		}

		void setRoughInterscet(const InersectCheckFunc& interscetRoughJudger){
			this->interscetRoughFunc = interscetRoughJudger;
		}

		void setPointInterscet(const InersectPointCheckFunc& interscetPointJudger){
			this->interscetPointFunc = interscetPointJudger;
		}

		explicit QuadTree(const unsigned int maxCount){
			items.reserve(maxCount);
		}

		explicit QuadTree(const Rect& boundary) : boundary(boundary){
			items.reserve(maximumItemCount);
		}

		QuadTree(const Rect& boundary, const unsigned int maxCount,
		         const InersectCheckFunc& interectRoughJudger,
		         const InersectCheckFunc& interectExactJudger,
		         const InersectPointCheckFunc& interectPointJudger
		) :
			interscetExactFunc(interectExactJudger),
			interscetRoughFunc(interectRoughJudger),
			interscetPointFunc(interectPointJudger),
			boundary(boundary), maximumItemCount(maxCount){
			this->items.reserve(maxCount * 2);
		}

		[[nodiscard]] Rect& getBoundary() noexcept{
			return boundary;
		}

		void setBoundary(const Rect& boundary){
			if(!allChildrenEmpty()) throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

			this->boundary = boundary;
		}

		void setBoundary(const T x, const T y, const T width, const T height){
			if(!allChildrenEmpty()) throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

			this->boundary.set(x, y, width, height);
		}

		void setBoundary(const T width, const T height){
			if(!allChildrenEmpty()) throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

			this->boundary.setSize(width, height);
		}

		bool remove(ItemTy& box){
			bool result;
			if(isLeaf()){
				std::unique_lock guard{itemsMtx};
				result = static_cast<bool>(std::erase_if(items, [ptr = &box](const std::reference_wrapper<ItemTy>& o){
					return &o.get() == ptr;
				}));
			} else{
				if(QuadTree* child = this->getFittingChild(this->obtainBound(box))){
					std::shared_lock guard{leafMtx};
					result = child->remove(box);
				} else{
					std::unique_lock guard{itemsMtx};
					result = static_cast<bool>(std::erase_if(items, [ptr = &box](const std::reference_wrapper<ItemTy>& o){
						return &o.get() == ptr;
					}));
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

		[[nodiscard]] bool isLeaf() const noexcept{
			return leaf;
		}

		[[nodiscard]] bool allChildrenEmpty() const noexcept{
			return currentSize == items.size();
		}

		template <Concepts::Invokable<void(QuadTree&)> Func>
		void each(Func&& func){
			func(*this);

			if(!isLeaf()){
				topLeft->each(std::forward<Func>(func));
				topRight->each(std::forward<Func>(func));
				bottomLeft->each(std::forward<Func>(func));
				bottomRight->each(std::forward<Func>(func));
			}
		}

		bool intersectAny(ItemTy& object){
			if(!this->inbound(object)){
				return false;
			}

			if(std::shared_lock _{itemsMtx}; std::ranges::any_of(items, std::bind_front(&QuadTree::isInersectedBetween, this, std::ref(object)))){
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
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

			if(std::shared_lock _{itemsMtx}; std::ranges::any_of(items, std::bind_front(&QuadTree::intersectWith, this, point))){
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
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

		/**
		 * @brief Notice that this may have side effects!
		 */
		void intersectAll(const ItemTy& object){
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
				this->topLeft->intersectAll(object);
				this->topRight->intersectAll(object);
				this->bottomLeft->intersectAll(object);
				this->bottomRight->intersectAll(object);
			}

			for(std::shared_lock _{itemsMtx}; auto& element : items){
				this->isInersectedBetween(object, element);
			}
		}

		template <Concepts::Invokable<void(ItemTy&)> Func>
		void intersect(const ItemTy& object, Func&& func){
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
				topLeft->intersect(object, func);
				topRight->intersect(object, func);
				bottomLeft->intersect(object, func);
				bottomRight->intersect(object, func);
			}

			for(std::shared_lock _{itemsMtx}; const auto& cont : items){
				if(this->isInersectedBetween(object, cont)){
					func(cont);
				}
			}
		}

		template <Concepts::Invokable<void(ItemTy&, const Rect&)> Func>
		void intersectRect(const Rect& rect, Func&& func){
			if(!this->inbound(rect)) return;

			//If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
				topLeft->intersectRect(rect, func);
				topRight->intersectRect(rect, func);
				bottomLeft->intersectRect(rect, func);
				bottomRight->intersectRect(rect, func);
			}

			for(std::shared_lock _{itemsMtx}; const auto& cont : items){
				func(cont, rect);
			}
		}

		template <
			typename Region,
			Concepts::Invokable<bool(const Rect&, const Region&)> Check,
			Concepts::Invokable<void(ItemTy&, const Region&)> Pred>
			requires !std::same_as<Region, Rect>
		void intersectRegion(const Region& region, Check&& boundCheck, Pred&& pred){
			if(!boundCheck(boundary, region)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
				topLeft->template intersectRegion<Region>(region, boundCheck, pred);
				topRight->template intersectRegion<Region>(region, boundCheck, pred);
				bottomLeft->template intersectRegion<Region>(region, boundCheck, pred);
				bottomRight->template intersectRegion<Region>(region, boundCheck, pred);
			}

			for(std::shared_lock _{itemsMtx}; const auto& cont : items){
				if(boundCheck(this->obtainBound(cont), region))pred(cont, region);
			}
		}

		template <Concepts::Invokable<void(ItemTy&)> Pred>
		void intersectPoint(const Vec2 point, Pred&& pred){
			if(!this->inbound(point)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
				topLeft->intersectPoint(point, std::forward<Pred>(pred));
				topRight->intersectPoint(point, std::forward<Pred>(pred));
				bottomLeft->intersectPoint(point, std::forward<Pred>(pred));
				bottomRight->intersectPoint(point, std::forward<Pred>(pred));
			}

			for(std::shared_lock _{itemsMtx}; const auto& cont : items){
				if(this->intersectWith(point, cont)){
					pred(cont);
				}
			}
		}

		template <Concepts::Invokable<void(ItemTy*)> Pred>
		void within(const ItemTy& object, const T dst, Pred&& pred){
			if(!this->withinBound(object, dst)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
				topLeft->within(object, dst, pred);
				topRight->within(object, dst, pred);
				bottomLeft->within(object, dst, pred);
				bottomRight->within(object, dst, pred);
			}

			for(std::shared_lock _{itemsMtx}; const auto& cont : items){
				pred(cont);
			}
		}

		template <Concepts::Invokable<void(ItemTy*)> Pred>
		bool intersectOnce(const ItemTy& object, Pred&& pred){
			if(!this->inbound(object)) return false;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(std::shared_lock _{leafMtx}; !isLeaf()){
				return topLeft->intersect(object, pred) ||
					topRight->intersect(object, pred) ||
					bottomLeft->intersect(object, pred) ||
					bottomRight->intersect(object, pred);
			}

			bool intersected = false;

			for(std::shared_lock _{itemsMtx}; const auto& cont : items){
				if(this->isInersectedBetween(object, cont)){
					intersected = true;
					pred(cont);
					break;
				}
			}

			return intersected;
		}

		bool intersectWith(const Vec2 point, const ItemTy& object) const{
			return this->obtainBound(object).containsPos_edgeExclusive(point) &&
				(!interscetPointFunc || interscetPointFunc(object, point));
		}

		bool inbound(const Rect& object) const{
			if(strict) return boundary.contains(object);
			return boundary.overlap(object);
		}

		bool inbound(const ItemTy& object) const{
			if(strict) return boundary.contains(this->obtainBound(object));
			return boundary.overlap(this->obtainBound(object));
		}

		bool inbound(const Vec2 object) const{
			return boundary.containsPos_edgeExclusive(object);
		}

		bool withinBound(const ItemTy& object, const T dst) const{
			return this->obtainBound(object).getCenter().within(boundary.getCenter(), dst);
		}

		/**
		 * @brief Notice that there is no unique check!
		 * @return whether this box is inserted
		 */
		bool insert(ItemTy& box){
			// Ignore objects that do not belong in this quad tree
			if(!this->inbound(box)){
				return false;
			}

			//If this box is inbound, it should must be added.
			++currentSize;

			// Otherwise, subdivide and then add the object to whichever node will accept it
			if(isLeaf()){
				if(items.size() >= maximumItemCount){
					split();
				} else{
					std::unique_lock guard{itemsMtx};
					items.push_back(box);
					return true;
				}
			}

			const Rect& rect = this->obtainBound(box);

			if(QuadTree* child = this->getFittingChild(rect)){
				return child->insert(box);
			}

			//Fallback
			std::unique_lock guard{itemsMtx};
			items.push_back(box);
			return true;
		}

		void clear(){
			{
				std::unique_lock guard{leafMtx};
				topLeft.reset(nullptr);
				topRight.reset(nullptr);
				bottomLeft.reset(nullptr);
				bottomRight.reset(nullptr);
				leaf = true;
			}

			currentSize = 0;

			std::unique_lock guard{itemsMtx};
			items.clear();
		}

		void clearItemsOnly(){
			{
				const bool _isLeaf = leaf.load();
				leaf = true;
				currentSize = 0;

				if(!_isLeaf){
					topLeft->clearItemsOnly();
					topRight->clearItemsOnly();
					bottomLeft->clearItemsOnly();
					bottomRight->clearItemsOnly();
				}
			}

			std::unique_lock guard{itemsMtx};
			items.clear();
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

			if(std::unique_lock guard{leafMtx}; topLeft != nullptr){
				topLeft->setBoundary(tl);
				topRight->setBoundary(tr);
				bottomLeft->setBoundary(bl);
				bottomRight->setBoundary(br);
			}else{
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
				std::unique_lock _{itemsMtx};
				std::ranges::copy(topLeft->items, std::back_inserter(items));
				std::ranges::copy(topRight->items, std::back_inserter(items));
				std::ranges::copy(bottomLeft->items, std::back_inserter(items));
				std::ranges::copy(bottomRight->items, std::back_inserter(items));
			}

			std::unique_lock l{leafMtx};
			topLeft->clearItemsOnly();
			topRight->clearItemsOnly();
			bottomLeft->clearItemsOnly();
			bottomRight->clearItemsOnly();
		}

		QuadTree* getFittingChild(const Rect boundingBox) const{
			if(isLeaf()) return nullptr;
			auto [midX, midY] = boundary.getCenter();

			// Object can completely fit within the top quadrants
			const bool topQuadrant = boundingBox.getSrcY() > midY;
			// Object can completely fit within the bottom quadrants
			const bool bottomQuadrant =
				boundingBox.getSrcY() < midY &&
				boundingBox.getSrcY() + boundingBox.getHeight() < midY;

			// Object can completely fit within the left quadrants
			if(boundingBox.getSrcX() < midX && boundingBox.getSrcX() + boundingBox.getWidth() <
				midX){
				if(topQuadrant){
					return topLeft.get();
				} else if(bottomQuadrant){
					return bottomLeft.get();
				}
			} else if(boundingBox.getSrcX() > midX){
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
