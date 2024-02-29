// ReSharper disable CppDFAUnreachableCode
module;

export module Geom.QuadTree;

export import Geom.Shape.Rect_Orthogonal;

import <vector>;
import Concepts;
import RuntimeException;
import <algorithm>;
import <functional>;
import <memory>;
import <mutex>;

using Geom::Shape::Rect_Orthogonal;

export namespace Geom {
	template <typename Cont, Concepts::Number T>
	class QuadTree {
		using Rect = Rect_Orthogonal<T>;

	public:
		using SubTree = std::array<QuadTree *, 4>;
		using Obtainer = std::function<const Rect&(const Cont*)>;
		using InersectCheck = std::function<bool(const Cont*, const Cont*)>;

	protected:
		Obtainer transformer{ nullptr };
		InersectCheck interscetExactFunc{ nullptr };
		InersectCheck interscetRoughFunc{ nullptr };
		// The boundary of this node
		Rect boundary{};

		std::mutex containerlock{};
		std::mutex subTreelock{};

		bool isInersectedBetween(const Cont* subject, const Cont* object) {
			if(subject == object) return false;
			bool intersected = obtainBound(subject).overlap(obtainBound(object));

			intersected &= !interscetRoughFunc || interscetRoughFunc(subject, object);
			intersected &= !interscetExactFunc || interscetExactFunc(subject, object);

			return intersected;
		}

		const Rect& obtainBound(const Cont* const cont) {
			return transformer(cont);
		}

		const Rect& obtainBound(const Cont& cont) {
			return transformer(&cont);
		}

		// The rectangles in this node
		std::vector<const Cont *> rectangles{};

		// The four children of this node
		std::unique_ptr<QuadTree> topLeft{ nullptr };
		std::unique_ptr<QuadTree> topRight{ nullptr };
		std::unique_ptr<QuadTree> bottomLeft{ nullptr };
		std::unique_ptr<QuadTree> bottomRight{ nullptr };

		unsigned int maximumItemCount = 4;
		bool strict                   = false;

		std::atomic_uint currentSize = 0;
		std::atomic_bool leaf        = true;

	public:
		[[nodiscard]] unsigned int size() const {
			return currentSize;
		}

		void setExactInterscet(const InersectCheck& interscetExactJudger) {
			this->interscetExactFunc = interscetExactJudger;
		}

		void setRoughInterscet(const InersectCheck& interscetRoughJudger) {
			this->interscetExactFunc = interscetRoughJudger;
		}

		explicit QuadTree(unsigned int maxCount) {
			rectangles.reserve(maxCount);
		}

		QuadTree(const Rect& boundary, const Obtainer& transformer) : transformer(transformer), boundary(boundary) {
			rectangles.reserve(maximumItemCount);
		}

		QuadTree(const Rect& boundary, const Obtainer& transformer, const InersectCheck& interectExactJudger,
		         unsigned int maxCount) : transformer(transformer), interscetExactFunc(interectExactJudger),
			boundary(boundary), maximumItemCount(maxCount) {
			rectangles.reserve(maxCount);
		}

		[[nodiscard]] Rect& getBoundary() {
			return boundary;
		}

		void setBoundary(const Rect& boundary) {
			if(!allChildrenEmpty()) throw ext::RuntimeException{ "QuadTree Boundary Should be set initially" };

			this->boundary = boundary;
		}

		void setBoundary(const T x, const T y, const T width, const T height) {
			this->boundary.set(x, y, width, height);
		}

		void setBoundary(const T width, const T height) {
			if(!allChildrenEmpty()) throw ext::RuntimeException{ "QuadTree Boundary Should be set initially" };

			this->boundary.setSize(width, height);
		}

		bool remove(const Cont* box) {
			bool result;
			if(isLeaf()) {
				containerlock.lock();
				result = static_cast<bool>(std::erase(rectangles, box));
				containerlock.unlock();
			} else {
				if(QuadTree* child = this->getFittingChild(this->obtainBound(box))) {
					result = child->remove(box);
				} else {
					containerlock.lock();
					result = static_cast<bool>(std::erase(rectangles, box));
					containerlock.unlock();
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

		template <Concepts::Invokable<void(QuadTree*)> Func>
		void each(Func&& func) {
			func(this);

			if(!isLeaf()) {
				topLeft->each(std::forward<Func>(func));
				topRight->each(std::forward<Func>(func));
				bottomLeft->each(std::forward<Func>(func));
				bottomRight->each(std::forward<Func>(func));
			}
		}

		bool intersectAny(const Cont* object) {
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

			std::ranges::for_each(rectangles, [this, object, pred = std::forward<Pred>(pred)](Cont* cont) {
				if(this->isInersectedBetween(object, cont)) {
					pred(cont);
				}
			});
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

			std::ranges::for_each(rectangles, [this, object, dst, pred = std::forward<Pred>(pred)](Cont* cont) {
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

			std::ranges::for_each(rectangles, [this, object, &pred, &intersected](Cont* cont) {
				if(intersected) return;
				if(this->isInersectedBetween(object, cont)) {
					intersected = true;
					pred(cont);
				}
			});

			return intersected;
		}

		bool inbound(const Cont* object) {
			if(strict) return boundary.contains(obtainBound(object));
			return boundary.overlap(obtainBound(object));
		}

		bool withinBound(const Cont* object, const T dst) {
			return obtainBound(object).getCenter().within(boundary.getCenter(), dst);
		}

		bool insert(const Cont* box) {
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
					std::lock_guard guard{ containerlock };
					rectangles.push_back(box);
					return true;
				}
			}

			const Rect& rect = this->obtainBound(box);
			// Add to relevant child, or root if can't fit completely in a child
			if(QuadTree* child = this->getFittingChild(rect)) {
				return child->insert(box);
			}

			//Fallback
			std::lock_guard guard{ containerlock };
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
			if(topLeft != nullptr) return;

			subTreelock.lock();

			const T x = boundary.getSrcX();
			const T y = boundary.getSrcY();
			const T w = boundary.getWidth() / static_cast<T>(2);
			const T h = boundary.getHeight() / static_cast<T>(2);

			const Rect bl{ x, y, w, h };
			const Rect br{ x + w, y, w, h };
			const Rect tl{ x, y + h, w, h };
			const Rect tr{ x + w, y + h, w, h };

			topLeft     = std::make_unique<QuadTree>(tl, transformer, interscetExactFunc, maximumItemCount);
			topRight    = std::make_unique<QuadTree>(tr, transformer, interscetExactFunc, maximumItemCount);
			bottomLeft  = std::make_unique<QuadTree>(bl, transformer, interscetExactFunc, maximumItemCount);
			bottomRight = std::make_unique<QuadTree>(br, transformer, interscetExactFunc, maximumItemCount);

			subTreelock.unlock();
		}

		void unsplit() {
			if(isLeaf()) return;
			leaf = true;

			containerlock.lock();
			rectangles.insert(rectangles.end(), std::make_move_iterator(topLeft->rectangles.begin()),
			                  std::make_move_iterator(topLeft->rectangles.end()));
			rectangles.insert(rectangles.end(), std::make_move_iterator(topRight->rectangles.begin()),
			                  std::make_move_iterator(topRight->rectangles.end()));
			rectangles.insert(rectangles.end(), std::make_move_iterator(bottomLeft->rectangles.begin()),
			                  std::make_move_iterator(bottomLeft->rectangles.end()));
			rectangles.insert(rectangles.end(), std::make_move_iterator(bottomRight->rectangles.begin()),
			                  std::make_move_iterator(bottomRight->rectangles.end()));
			containerlock.unlock();

			topLeft->clearItemsOnly();
			topRight->clearItemsOnly();
			bottomLeft->clearItemsOnly();
			bottomRight->clearItemsOnly();
		}

		QuadTree* getFittingChild(const Rect& boundingBox) {
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

	template <typename T>
	using QuadTreeF = QuadTree<T, float>;
}
