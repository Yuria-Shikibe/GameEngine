// ReSharper disable CppDFAUnreachableCode
module;

export module Geom.QuadTreeBrief;

export import Geom.Rect_Orthogonal;
export import Geom.Vector2D;

import Concepts;
import RuntimeException;
import std;

using Geom::Rect_Orthogonal;

//TODO The design is so bad!
export namespace Geom {
	template <typename Cont, Concepts::Number T, Concepts::InvokeNullable<Rect_Orthogonal<T>(const Cont&)> auto transformer>
	class QuadTreeBrief {

	public:
		using Rect = Rect_Orthogonal<T>;
		using ValueType = Cont;

		static constexpr bool ValidTrans = !std::same_as<std::nullptr_t, decltype(transformer)>;
		static_assert(ValidTrans, "Invalid Trans Func!");

	protected:
		// The boundary of this node
		Rect boundary{};

		bool isInersectedBetween(const Cont* subject, const Cont* object) {
			if(subject == object) return false;
			const bool intersected =
				this->obtainBound(subject).overlap(this->obtainBound(object));

			return intersected;
		}

		static Rect obtainBound(const Cont* const cont) {
			return transformer(*cont);
		}

		static Rect obtainBound(const Cont& cont) {
			return transformer(cont);
		}

		// The rectangles in this node
		std::vector<Cont *> rectangles{};

		// The four children of this node
		std::unique_ptr<QuadTreeBrief> topLeft{ nullptr };
		std::unique_ptr<QuadTreeBrief> topRight{ nullptr };
		std::unique_ptr<QuadTreeBrief> bottomLeft{ nullptr };
		std::unique_ptr<QuadTreeBrief> bottomRight{ nullptr };

		unsigned int maximumItemCount = 4;
		bool strict = false;

		unsigned currentSize = 0;
		bool leaf = true;

	public:
		QuadTreeBrief() = default;

		[[nodiscard]] unsigned int size() const {
			return currentSize;
		}

		explicit QuadTreeBrief(unsigned int maxCount) {
			rectangles.reserve(maxCount);
		}

		explicit QuadTreeBrief(const Rect boundary) : boundary(boundary) {
			rectangles.reserve(maximumItemCount);
		}

		QuadTreeBrief(const Rect boundary, unsigned int maxCount) :
			boundary(boundary), maximumItemCount(maxCount) {
			this->rectangles.reserve(maxCount);
		}

		[[nodiscard]] Rect& getBoundary() {
			return boundary;
		}

		[[nodiscard]] const Rect& getBoundary() const {
			return boundary;
		}

		void setBoundary(const Rect boundary) {
			if(!allChildrenEmpty()) throw ext::RuntimeException{ "QuadTreeBrief Boundary Should be set initially" };

			this->boundary = boundary;
		}

		void setBoundary(const T x, const T y, const T width, const T height) {
			this->boundary.set(x, y, width, height);
		}

		void setBoundary(const T width, const T height) {
			if(!allChildrenEmpty()) throw ext::RuntimeException{ "QuadTreeBrief Boundary Should be set initially" };

			this->boundary.setSize(width, height);
		}

		bool remove(Cont* box) {
			bool result;
			if(isLeaf()) {
				result = static_cast<bool>(std::erase(rectangles, box));
			} else {
				if(QuadTreeBrief* child = this->getFittingChild(this->obtainBound(box))) {
					result = child->remove(box);
				} else {
					result = static_cast<bool>(std::erase(rectangles, box));
				}

				if(currentSize <= maximumItemCount) unsplit();
			}

			if(result) {
				--currentSize;
				if(allChildrenEmpty()) {
					leaf = true;
				}
			}

			return result;
		}

		[[nodiscard]] bool isLeaf() const {
			return leaf;
		}

		[[nodiscard]] bool allChildrenEmpty() const {
			return currentSize == rectangles.size();
		}

		template <Concepts::Invokable<void(QuadTreeBrief*)> Func>
		void each(Func&& func) {
			func(this);

			if(!isLeaf()) {
				topLeft->each(std::forward<Func>(func));
				topRight->each(std::forward<Func>(func));
				bottomLeft->each(std::forward<Func>(func));
				bottomRight->each(std::forward<Func>(func));
			}
		}

		template <Concepts::Invokable<void(const QuadTreeBrief*)> Func>
		void each(Func&& func) const {
			func(this);

			if(!isLeaf()) {
				topLeft->each(std::forward<Func>(func));
				topRight->each(std::forward<Func>(func));
				bottomLeft->each(std::forward<Func>(func));
				bottomRight->each(std::forward<Func>(func));
			}
		}

		bool intersectAny(Cont* object) {
			if(!this->inbound(object)) {
				return false;
			}

			if(std::ranges::any_of(rectangles, [this, object](const Cont* cont) {
				return this->isInersectedBetween(object, cont);
			})) {
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
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

		[[nodiscard]] const std::vector<Cont*>& getChildren() const{ return rectangles; }

		Cont* getIntersectAny(const Vec2 point) const {
			if(!this->inbound(point)) {
				return nullptr;
			}

			for (const auto cont : rectangles){
				if(this->intersectWith(cont, point)){
					return cont;
				}
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
				Cont* cur{nullptr};
				if((cur = topLeft->getIntersectAny(point)))return cur; // NOLINT(*-assignment-in-if-condition)
				if((cur = topRight->getIntersectAny(point)))return cur; // NOLINT(*-assignment-in-if-condition)
				if((cur = bottomLeft->getIntersectAny(point)))return cur; // NOLINT(*-assignment-in-if-condition)
				if((cur = bottomRight->getIntersectAny(point)))return cur; // NOLINT(*-assignment-in-if-condition)
			}

			// Otherwise, the rectangle does not overlap with any rectangle in the quad tree
			return nullptr;
		}

		bool intersectAny(const Vec2 point) {
			if(!this->inbound(point)) {
				return false;
			}

			if(std::ranges::any_of(rectangles, [this, point](const Cont* cont) {
				return this->intersectWith(cont, point);
			})) {
				return true;
			}

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
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

		void intersectAll(const Cont* object) {
			if(!this->inbound(object))return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
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
		void intersect(const Cont* object, Pred&& pred) {
			if(!this->inbound(object)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
				topLeft->intersect(object, pred);
				topRight->intersect(object, pred);
				bottomLeft->intersect(object, pred);
				bottomRight->intersect(object, pred);
			}

			for (const auto cont : rectangles){
				if(this->isInersectedBetween(object, cont)) {
					pred(cont);
				}
			}
		}

		template <typename Rect>
		void intersectRect(const Rect& rect,
				Concepts::Invokable<bool(const Cont*, const Rect&)> auto && check,
				Concepts::Invokable<void(Cont*)> auto && pred) {
			if(!this->inbound(rect)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
				topLeft->intersectRect(rect, std::forward<decltype(check)>(check), std::forward<decltype(pred)>(pred));
				topRight->intersectRect(rect, std::forward<decltype(check)>(check), std::forward<decltype(pred)>(pred));
				bottomLeft->intersectRect(rect, std::forward<decltype(check)>(check), std::forward<decltype(pred)>(pred));
				bottomRight->intersectRect(rect, std::forward<decltype(check)>(check), std::forward<decltype(pred)>(pred));
			}

			for (const auto cont : rectangles){
				if(check(cont, rect)) {
					pred(cont);
				}
			}
		}

		template <Concepts::Invokable<void(Cont*)> Pred>
		void intersectPoint(const Vec2 point, Pred&& pred) {
			if(!this->inbound(point)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
				topLeft->intersectPoint(point, std::forward<Pred>(pred));
				topRight->intersectPoint(point, std::forward<Pred>(pred));
				bottomLeft->intersectPoint(point, std::forward<Pred>(pred));
				bottomRight->intersectPoint(point, std::forward<Pred>(pred));
			}

			for (const auto cont : rectangles){
				if(this->intersectWith(cont, point)) {
					pred(cont);
				}
			}
		}

		template <Concepts::Invokable<void(Cont*)> Pred>
		void within(const Cont* object, const T dst, Pred&& pred) {
			if(!this->withinBound(object, dst)) return;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
				topLeft->within(object, dst, pred);
				topRight->within(object, dst, pred);
				bottomLeft->within(object, dst, pred);
				bottomRight->within(object, dst, pred);
			}

			std::ranges::for_each(rectangles, [this, object, dst, pred = std::forward<Pred>(pred)](const Cont* cont) {
				if(this->obtainBound(cont).getCenter().within(this->obtainBound(object), dst)) {
					pred(cont);
				}
			});
		}

		template <Concepts::Invokable<void(Cont*)> Pred>
		bool intersectOnce(const Cont* object, Pred&& pred) {
			if(!this->inbound(object)) return false;

			// If this node has children, check if the rectangle overlaps with any rectangle in the children
			if(!isLeaf()) {
				return topLeft->intersect(object, pred) ||
				       topRight->intersect(object, pred) ||
				       bottomLeft->intersect(object, pred) ||
				       bottomRight->intersect(object, pred);
			}

			bool intersected = false;

			std::ranges::for_each(rectangles, [this, object, pred = std::forward<Pred>(pred), &intersected](Cont* cont) {
				if(intersected) return;
				if(this->isInersectedBetween(object, cont)) {
					intersected = true;
					pred(cont);
				}
			});

			return intersected;
		}

		bool intersectWith(const Cont* object, const Vec2 point) const {
			return this->obtainBound(object).containsPos_edgeExclusive(point);
		}

		bool inbound(const Rect& rect) const {
			if(strict) return boundary.contains(rect);
			return boundary.overlap(rect);
		}

		bool inbound(const Cont* object) const {
			if(strict) return boundary.contains(obtainBound(object));
			return boundary.overlap(obtainBound(object));
		}

		bool inbound(const Vec2 object) const {
			return boundary.containsPos_edgeExclusive(object);
		}

		bool withinBound(const Cont* object, const T dst) {
			return obtainBound(object).getCenter().within(boundary.getCenter(), dst);
		}

		bool insert(Cont* box) {
			// Ignore objects that do not belong in this quad tree
			if(!this->inbound(box)) {
				return false;
			}

			//If this box is inbound, it should must be added.
			++currentSize;

			// Otherwise, subdivide and then add the object to whichever node will accept it
			if(isLeaf()) {
				if(rectangles.size() >= maximumItemCount) {
					split();
				} else {
					rectangles.push_back(box);
					return true;
				}
			}

			const Rect& rect = this->obtainBound(box);
			// Add to relevant child, or root if can't fit completely in a child
			if(QuadTreeBrief* child = this->getFittingChild(rect)) {
				return child->insert(box);
			}

			//Fallback
			rectangles.push_back(box);
			return true;
		}

		void clear() {
			topLeft.reset(nullptr);
			topRight.reset(nullptr);
			bottomLeft.reset(nullptr);
			bottomRight.reset(nullptr);

			currentSize = 0;

			rectangles.clear();
		}

		void clearItemsOnly() {
			if(!isLeaf()) {
				topLeft->clearItemsOnly();
				topRight->clearItemsOnly();
				bottomLeft->clearItemsOnly();
				bottomRight->clearItemsOnly();
			}

			leaf        = true;
			currentSize = 0;
			rectangles.clear();
		}

		void split() {
			if(!isLeaf()) return;
			leaf = false;

			const T x = boundary.getSrcX();
			const T y = boundary.getSrcY();
			const T w = boundary.getWidth() / static_cast<T>(2);
			const T h = boundary.getHeight() / static_cast<T>(2);

			const Rect bl{ x, y, w, h };
			const Rect br{ x + w, y, w, h };
			const Rect tl{ x, y + h, w, h };
			const Rect tr{ x + w, y + h, w, h };

			if(topLeft != nullptr){
				topLeft    ->setBoundary(tl);
				topRight   ->setBoundary(tr);
				bottomLeft ->setBoundary(bl);
				bottomRight->setBoundary(br);
			}else{
				topLeft     = std::make_unique<QuadTreeBrief>(tl, maximumItemCount);
				topRight    = std::make_unique<QuadTreeBrief>(tr, maximumItemCount);
				bottomLeft  = std::make_unique<QuadTreeBrief>(bl, maximumItemCount);
				bottomRight = std::make_unique<QuadTreeBrief>(br, maximumItemCount);
			}
		}

		void unsplit() {
			if(isLeaf()) return;
			leaf = true;

			{
				std::copy(topLeft->rectangles.begin(), topLeft->rectangles.end(), std::back_inserter(rectangles));
				std::copy(topRight->rectangles.begin(), topLeft->rectangles.end(), std::back_inserter(rectangles));
				std::copy(bottomLeft->rectangles.begin(), topLeft->rectangles.end(), std::back_inserter(rectangles));
				std::copy(bottomRight->rectangles.begin(), topLeft->rectangles.end(), std::back_inserter(rectangles));
			}

			topLeft->clearItemsOnly();
			topRight->clearItemsOnly();
			bottomLeft->clearItemsOnly();
			bottomRight->clearItemsOnly();
		}

		QuadTreeBrief* getFittingChild(const Rect boundingBox) {
			if(isLeaf()) return nullptr;
			const T verticalMidpoint   = boundary.getSrcX() + boundary.getWidth() / 2;
			const T horizontalMidpoint = boundary.getSrcY() + boundary.getHeight() / 2;

			// Object can completely fit within the top quadrants
			const bool topQuadrant = boundingBox.getSrcY() > horizontalMidpoint;
			// Object can completely fit within the bottom quadrants
			const bool bottomQuadrant = boundingBox.getSrcY() < horizontalMidpoint && (
				                            boundingBox.getSrcY() + boundingBox.getHeight()) < horizontalMidpoint;

			// Object can completely fit within the left quadrants
			if(boundingBox.getSrcX() < verticalMidpoint && boundingBox.getSrcX() + boundingBox.getWidth() <
			   verticalMidpoint) {
				if(topQuadrant) {
					return topLeft.get();
				} else if(bottomQuadrant) {
					return bottomLeft.get();
				}
			} else if(boundingBox.getSrcX() > verticalMidpoint) {
				// Object can completely fit within the right quadrants
				if(topQuadrant) {
					return topRight.get();
				} else if(bottomQuadrant) {
					return bottomRight.get();
				}
			}

			return nullptr;
		}
	};

	template <typename T, auto func>
	using QuadTreeBriefF = QuadTreeBrief<T, float, func>;
}
