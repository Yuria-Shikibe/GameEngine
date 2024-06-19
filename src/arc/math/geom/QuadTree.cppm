// ReSharper disable CppDFAUnreachableCode
module;

export module Geom.QuadTree;

export import Geom.Rect_Orthogonal;
export import Geom.Vector2D;

import ext.Concepts;
import ext.RuntimeException;
import std;
export import Geom.QuadTree.Interface;

namespace Geom{
	template <bool enable>
	struct SharedGuard : std::shared_lock<std::shared_mutex>{
		using std::shared_lock<std::shared_mutex>::shared_lock;
	};

	template <>
	struct SharedGuard<false>{
		[[nodiscard]] constexpr SharedGuard() noexcept = default;

		[[nodiscard]] explicit constexpr SharedGuard(const auto&) noexcept {};
	};

	template <bool enable>
	struct UniqueGuard : std::unique_lock<std::shared_mutex>{
		using std::unique_lock<std::shared_mutex>::unique_lock;
	};

	template <>
	struct UniqueGuard<false>{};

	template <bool enable, bool strict>
	class MtxBase{
		static constexpr bool EnableShared = enable && strict;

		using MtxTy = std::conditional_t<strict, std::shared_mutex, std::mutex>;
		using UniqueLockTy = std::conditional_t<strict, UniqueGuard<true>, std::scoped_lock<MtxTy>>;

		mutable MtxTy itemsMtx{};
		mutable MtxTy leafMtx{};

	public:
		auto getItemGruard_Shared() const{
			return SharedGuard<EnableShared>{itemsMtx};
		}

		auto getLeafGruard_Shared() const{
			return SharedGuard<EnableShared>{leafMtx};
		}

		auto getItemGruard_Unique() const{
			return UniqueLockTy{itemsMtx};
		}

		auto getLeafGruard_Unique() const{
			return UniqueLockTy{leafMtx};
		}
	};

	template <>
	class MtxBase<false, false>{
	public:
		static SharedGuard<false> getItemGruard_Shared(){
			return {};
		}

		static SharedGuard<false> getLeafGruard_Shared(){
			return {};
		}

		static UniqueGuard<false> getItemGruard_Unique(){
			return {};
		}

		static UniqueGuard<false> getLeafGruard_Unique(){
			return {};
		}
	};

	template <>
	class [[deprecated("Should be Sync & Strict")]] MtxBase<false, true>;

	// using ItemTy = int;
	// using T = float;
	// constexpr bool sync = false;

	export
	template <typename ItemTy, Concepts::Number T = float, bool sync = true, bool strict = false>
		requires Concepts::Derived<ItemTy, QuadTreeAdaptable<ItemTy, T>>
	class QuadTree : MtxBase<sync, strict>{
	public:
		using Rect = Geom::Rect_Orthogonal<T>;
		using ArithmeticType = T;
		using ValueType = ItemTy;

		using PointType = Geom::Vector2D<ArithmeticType>;

		static constexpr bool HasSyncLock = sync;

		static constexpr bool HasRoughIntersect = requires(ValueType value){
			{ value.roughIntersectWith(value) } -> std::same_as<bool>;
		};

		static constexpr bool HasExactIntersect = requires(ValueType value){
			{ value.exactIntersectWith(value) } -> std::same_as<bool>;
		};

		static constexpr bool HasPointIntersect = requires(ValueType value, PointType p){
			{ value.containsPoint(p) } -> std::same_as<bool>;
		};

	protected:
		// The boundary of this node
		Rect boundary{};



		bool isInersectedBetween(const ItemTy& subject, const ItemTy& object) const{
			//TODO equalTy support?
			if(&subject == &object) return false;

			// if constexpr(requires(ValueType value){
			// 	{ value == value } -> std::same_as<bool>;
			// }){
			// 	if(subject == object) return false;
			// } else{
			// 	if(&subject == &object) return false;
			// }

			bool intersected = this->obtainItemBound(subject).overlap(this->obtainItemBound(object));

			if constexpr(HasRoughIntersect){
				if(intersected) intersected &= subject.roughIntersectWith(object);
			}

			if constexpr(HasExactIntersect){
				if(intersected) intersected &= subject.exactIntersectWith(object);
			}

			return intersected;
		}

		static Rect obtainItemBound(const ItemTy& cont){
			static_assert(requires(ValueType value){
				{ value.getBound() } -> std::same_as<Rect>;
			}, "QuadTree Requires ValueType impl at `Rect getBound()` member function");

			return cont.getBound();
		}

		// The rectangles in this node
		std::vector<ItemTy*> items{};

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
		[[nodiscard]] QuadTree() = default;

		QuadTree(const QuadTree& other) = delete;

		QuadTree(QuadTree&& other) noexcept
			:
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

		explicit QuadTree(const unsigned int maxCount){
			this->items.reserve(maxCount * 2);
		}

		explicit QuadTree(const Rect& boundary) : boundary(boundary){
			this->items.reserve(maximumItemCount * 2);
		}

		QuadTree(const Rect& boundary, const unsigned int maxCount) :
			boundary(boundary), maximumItemCount(maxCount){
			this->items.reserve(maxCount * 2);
		}

		[[nodiscard]] Rect getBoundary() const noexcept{
			return boundary;
		}

		void setBoundary(const Rect& boundary){
			if(!allChildrenEmpty()){
				clearItemsOnly();
				//TODO always have problems here...
			}

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
				auto _ = this->getItemGruard_Unique();
				result = static_cast<bool>(std::erase_if(items, [ptr = &box](const std::reference_wrapper<ItemTy>& o){
					return &o.get() == ptr;
				}));
			} else{
				if(QuadTree* child = this->getFittingChild(this->obtainItemBound(box))){
					auto _ = this->getLeafGruard_Shared();
					result = child->remove(box);
				} else{
					auto _ = this->getItemGruard_Unique();
					result = static_cast<bool>(std::erase_if(items,
						[ptr = &box](const std::reference_wrapper<ItemTy>& o){
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
				topLeft->each(func);
				topRight->each(func);
				bottomLeft->each(func);
				bottomRight->each(func);
			}
		}

		bool intersectAny(ItemTy& object){
			if(!this->inbound(object)){
				return false;
			}

			if(auto _ = this->getItemGruard_Shared(); std::ranges::any_of(items, [this, &object](const ItemTy* o){
				return isInersectedBetween(object, *o);
			})){
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
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

			if(auto _ = this->getItemGruard_Shared(); std::ranges::any_of(items, [this, point](const ItemTy* o){
				return intersectWith(point, *o);
			})){
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
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
		template <
			Concepts::InvokeNullable<void(const ItemTy&, ItemTy&)> Func = std::nullptr_t,
			Concepts::InvokeNullable<bool(const ItemTy&, const ItemTy&)> Filter = std::nullptr_t>
		void intersectAll(const ItemTy& object, Func&& func = nullptr, Filter&& filter = nullptr) const{
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				this->topLeft->intersectAll(object);
				this->topRight->intersectAll(object);
				this->bottomLeft->intersectAll(object);
				this->bottomRight->intersectAll(object);
			}

			for(auto _ = this->getItemGruard_Shared(); const auto element : items){
				if(this->isInersectedBetween(object, *element)){
					if constexpr (!std::same_as<Func, std::nullptr_t>){
						if constexpr (!std::same_as<Filter, std::nullptr_t>){
							if(!filter(object, *element))continue;
						}

						func(object, *element);
					}
				}
			}
		}

		template <
			Concepts::InvokeNullable<void(ItemTy&, ItemTy&)> Func = std::nullptr_t,
			Concepts::InvokeNullable<bool(ItemTy&, const ItemTy&)> Filter = std::nullptr_t>
		void intersectAll(ItemTy& object, Func&& func = nullptr, Filter&& filter = nullptr) const{
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				this->topLeft->intersectAll(object);
				this->topRight->intersectAll(object);
				this->bottomLeft->intersectAll(object);
				this->bottomRight->intersectAll(object);
			}

			for(auto _ = this->getItemGruard_Shared(); const auto element : items){
				if(this->isInersectedBetween(object, *element)){
					if constexpr (!std::same_as<Func, std::nullptr_t>){
						if constexpr (!std::same_as<Filter, std::nullptr_t>){
							if(!filter(object, *element))continue;
						}

						func(object, *element);
					}
				}
			}
		}

		template <Concepts::Invokable<void(ItemTy&)> Func>
		void intersect(const ItemTy& object, Func&& func){
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				topLeft->intersect(object, func);
				topRight->intersect(object, func);
				bottomLeft->intersect(object, func);
				bottomRight->intersect(object, func);
			}

			for(auto _ = this->getItemGruard_Shared(); auto* cont : items){
				if(this->isInersectedBetween(object, *cont)){
					func(*cont);
				}
			}
		}


		template <Concepts::Invokable<void(ItemTy&, const Rect&)> Func>
		void intersectRect(const Rect& rect, Func&& func){
			if(!this->inbound(rect)) return;

			//If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				topLeft->intersectRect(rect, func);
				topRight->intersectRect(rect, func);
				bottomLeft->intersectRect(rect, func);
				bottomRight->intersectRect(rect, func);
			}

			for(auto _ = this->getItemGruard_Shared(); auto* cont : items){
				func(*cont, rect);
			}
		}

		template <typename Region>
			requires !std::same_as<Region, Rect>
		void intersectRegion(const Region& region,
			Concepts::Invokable<bool(const Rect&, const Region&)> auto&& boundCheck,
			Concepts::Invokable<void(ItemTy&, const Region&)> auto&& func){
			if(!boundCheck(boundary, region)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				topLeft->template intersectRegion<Region>(region, boundCheck, func);
				topRight->template intersectRegion<Region>(region, boundCheck, func);
				bottomLeft->template intersectRegion<Region>(region, boundCheck, func);
				bottomRight->template intersectRegion<Region>(region, boundCheck, func);
			}

			for(auto _ = this->getItemGruard_Shared(); auto* cont : items){
				if(boundCheck(this->obtainItemBound(*cont), region)) func(*cont, region);
			}
		}

		void intersectPoint(const Vec2 point, Concepts::Invokable<void(ItemTy&)> auto&& func){
			if(!this->inbound(point)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				topLeft->intersectPoint(point, func);
				topRight->intersectPoint(point, func);
				bottomLeft->intersectPoint(point, func);
				bottomRight->intersectPoint(point, func);
			}

			for(auto _ = this->getItemGruard_Shared(); auto* cont : items){
				if(this->intersectWith(point, *cont)){
					func(*cont);
				}
			}
		}

		template <Concepts::Invokable<void(ItemTy*)> Pred>
		void within(const ItemTy& object, const T dst, Pred&& pred){
			if(!this->withinBound(object, dst)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				topLeft->within(object, dst, pred);
				topRight->within(object, dst, pred);
				bottomLeft->within(object, dst, pred);
				bottomRight->within(object, dst, pred);
			}

			for(auto _ = this->getItemGruard_Shared(); const auto* cont : items){
				pred(*cont);
			}
		}

		template <Concepts::Invokable<void(ItemTy*)> Pred>
		bool intersectOnce(const ItemTy& object, Pred&& pred){
			if(!this->inbound(object)) return false;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(auto _ = this->getLeafGruard_Shared(); !isLeaf()){
				return topLeft->intersect(object, pred) ||
				       topRight->intersect(object, pred) ||
				       bottomLeft->intersect(object, pred) ||
				       bottomRight->intersect(object, pred);
			}

			bool intersected = false;

			for(auto _ = this->getItemGruard_Shared(); const auto* cont : items){
				if(this->isInersectedBetween(object, *cont)){
					intersected = true;
					pred(*cont);
					break;
				}
			}

			return intersected;
		}

		bool intersectWith(const Vec2 point, const ItemTy& object) const requires (HasPointIntersect){
			return this->obtainItemBound(object).containsPos_edgeExclusive(point) && object.containsPoint(point);
		}

		bool inbound(const Rect& object) const{
			if(strict) return boundary.contains(object);
			return boundary.overlap(object);
		}

		bool inbound(const ItemTy& object) const{
			if(strict) return boundary.contains(this->obtainItemBound(object));
			return boundary.overlap(this->obtainItemBound(object));
		}

		bool inbound(const Vec2 object) const{
			return boundary.containsPos_edgeExclusive(object);
		}

		bool withinBound(const ItemTy& object, const T dst) const{
			return this->obtainItemBound(object).getCenter().within(boundary.getCenter(), dst);
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
					auto _ = this->getItemGruard_Unique();
					items.push_back(&box);
					return true;
				}
			}

			const Rect& rect = this->obtainItemBound(box);

			if(QuadTree* child = this->getFittingChild(rect)){
				return child->insert(box);
			}

			//Fallback
			auto _ = this->getItemGruard_Unique();
			items.push_back(&box);
			return true;
		}

		void clear(){
			{
				auto _ = this->getLeafGruard_Unique();
				topLeft.reset(nullptr);
				topRight.reset(nullptr);
				bottomLeft.reset(nullptr);
				bottomRight.reset(nullptr);
				leaf = true;
			}

			currentSize = 0;

			auto _ = this->getItemGruard_Unique();
			items.clear();
		}

		void clearItemsOnly(){
			if(currentSize == 0) return;

			{
				auto _ = this->getLeafGruard_Unique();
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

			auto _ = this->getItemGruard_Unique();
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

			if(auto _ = this->getLeafGruard_Unique(); topLeft != nullptr){
				topLeft->setBoundary(tl);
				topRight->setBoundary(tr);
				bottomLeft->setBoundary(bl);
				bottomRight->setBoundary(br);
			} else{
				topLeft = std::make_unique<QuadTree>(tl, maximumItemCount);
				topRight = std::make_unique<QuadTree>(tr, maximumItemCount);
				bottomLeft = std::make_unique<QuadTree>(bl, maximumItemCount);
				bottomRight = std::make_unique<QuadTree>(br, maximumItemCount);
			}
		}

		void unsplit(){
			if(isLeaf()) return;
			leaf = true;

			{
				auto _ = this->getItemGruard_Unique();
				std::ranges::copy(topLeft->items, std::back_inserter(items));
				std::ranges::copy(topRight->items, std::back_inserter(items));
				std::ranges::copy(bottomLeft->items, std::back_inserter(items));
				std::ranges::copy(bottomRight->items, std::back_inserter(items));
			}

			auto _ = this->getLeafGruard_Unique();
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
				}

				if(bottomQuadrant){
					return bottomLeft.get();
				}
			} else if(boundingBox.getSrcX() > midX){
				// Object can completely fit within the right quadrants
				if(topQuadrant){
					return topRight.get();
				}

				if(bottomQuadrant){
					return bottomRight.get();
				}
			}

			return nullptr;
		}
	};
}
