
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

using Geom::Shape::Rect_Orthogonal;

template <typename T>
using Rect = Rect_Orthogonal<T>;

export namespace Geom {
    template <typename Cont, Concepts::Number T>
    class QuadTree {
    public:
        using obtainer = std::function<const Rect<T>&(const Cont&)>;
    protected:
        obtainer transformer{nullptr};
        // The boundary of this node
        Rect<T> boundary{};

        Rect<T>& obtainBound(const Cont* const cont) {
            return transformer(*cont);
        }

        Rect<T>& obtainBound(const Cont& cont) {
            return transformer(cont);
        }

        // The rectangles in this node
        std::vector<const Cont*> rectangles{};

        // The four children of this node
        std::unique_ptr<QuadTree> northwest{nullptr};
        std::unique_ptr<QuadTree> northeast{nullptr};
        std::unique_ptr<QuadTree> southwest{nullptr};
        std::unique_ptr<QuadTree> southeast{nullptr};

        unsigned int maximumItemCount = 4;

    public:
        explicit QuadTree(unsigned int maxCount){
            rectangles.reserve(maxCount);
        }

        QuadTree(const Rect<T>& boundary, const obtainer& transformer) : transformer(transformer), boundary(boundary) {
            rectangles.reserve(maximumItemCount);
        }

        QuadTree(const Rect<T>& boundary, const obtainer& transformer, unsigned int maxCount) : transformer(transformer), boundary(boundary), maximumItemCount(maxCount) {
            rectangles.reserve(maxCount);
        }

        [[nodiscard]] Rect<T>& getBoundary() {
            return boundary;
        }

        void setBoundary(const Rect<T>& boundary) {
            if(!allChildrenEmpty())throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

            this->boundary = boundary;
        }

        void setBoundary(const T x, const T y, const T width, const T height) {
            this->boundary.set(x, y, width, height);
        }

        void setBoundary(const T width, const T height) {
            if(!allChildrenEmpty())throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

            this->boundary.setSize(width, height);
        }

        // ReSharper disable once CppDFAConstantFunctionResult
        [[nodiscard]] bool overlap(const Cont& box) const {
            return overlap(obtainBound(box));
        }

        void remove(const Cont& box) {
            // Check if the rectangle is in this node
            auto it = std::find(rectangles.begin(), rectangles.end(), box);
            if (it != rectangles.end()) {
                // If the rectangle is in this node, remove it
                rectangles.erase(it);
            }else if (!isLeaf()) { // If this node has children, check if the rectangle is in the children
                northwest->remove(box);
                northeast->remove(box);
                southwest->remove(box);
                southeast->remove(box);
            }

            // Check if this node should be merged
            if (rectangles.empty() && allChildrenEmpty()) {
                clear();
            }
        }

        [[nodiscard]] bool isLeaf() const {
            return northwest == nullptr || northeast == nullptr || southeast == nullptr || southwest == nullptr;
        }

        [[nodiscard]] bool allChildrenEmpty() const {
            if (!isLeaf()) {
                if (!northwest->rectangles.empty() || !northeast->rectangles.empty() ||
                    !southwest->rectangles.empty() || !southeast->rectangles.empty()) {
                    return false;
                }
            }

            return true;
        }

        bool overlap(const Rect<T>& rectangle) {
            if(std::ranges::any_of(rectangles, [this, &rectangle](const Cont* cont) {
                    return rectangle.overlap(obtainBound(cont));
            })) {
                return true;
            }

            // If this node has children, check if the rectangle overlaps with any rectangle in the children
            if (!isLeaf()) {
                //TODO uses direction vector to get more possible results?
                if (northwest->overlap(rectangle)) return true;
                if (northeast->overlap(rectangle)) return true;
                if (southwest->overlap(rectangle)) return true;
                if (southeast->overlap(rectangle)) return true;
            }

            // Otherwise, the rectangle does not overlap with any rectangle in the quad tree
            return false;
        }

        bool insert(const Cont& box) {
            // Ignore objects that do not belong in this quad tree
            if (boundary.dissociated(obtainBound(box))) {
                return false;
            }

            // If there is space in this quad tree and if it does not subdivide, add the object here
            if (rectangles.size() < maximumItemCount) {
                rectangles.push_back(&box);
                return true;
            }

            // Otherwise, subdivide and then add the object to whichever node will accept it
            if (isLeaf()) {
                subDivide();
            }

            if (northwest->insert(box)) return true;
            if (northeast->insert(box)) return true;
            if (southwest->insert(box)) return true;
            if (southeast->insert(box)) return true;

            // Otherwise, the object cannot be inserted for some unknown reason (this should never happen)
            return false;
        }

        void clear() {
            northwest.reset(nullptr);
            northeast.reset(nullptr);
            southwest.reset(nullptr);
            southeast.reset(nullptr);

            rectangles.clear();
        }

        void clearItemsOnly() {
            if(!isLeaf()) {
                northwest->clearItemsOnly();
                northeast->clearItemsOnly();
                southwest->clearItemsOnly();
                southeast->clearItemsOnly();
            }

            rectangles.clear();
        }

        void subDivide() {
            const T x = boundary.getSrcX();
            const T y = boundary.getSrcY();
            const T w = boundary.getWidth() / static_cast<T>(2);
            const T h = boundary.getHeight() / static_cast<T>(2);

            const Rect<T> nw{x    , y    , w, h};
            const Rect<T> ne{x + w, y    , w, h};
            const Rect<T> sw{x    , y + h, w, h};
            const Rect<T> se{x + w, y + h, w, h};

            northwest = std::make_unique<QuadTree>(nw, transformer, maximumItemCount);
            northeast = std::make_unique<QuadTree>(ne, transformer, maximumItemCount);
            southwest = std::make_unique<QuadTree>(sw, transformer, maximumItemCount);
            southeast = std::make_unique<QuadTree>(se, transformer, maximumItemCount);
        }
    };

    template <typename T>
    using QuadTreeF = QuadTree<T, float>;
}
