// ReSharper disable CppDFAUnreachableCode
module;

export module Geom.QuadTree;

export import Geom.Shape.Rect_Orthogonal;

import <vector>;
import Concepts;
import RuntimeException;
import <algorithm>;
import <functional>;

using Geom::Shape::Rect_Orthogonal;

template <typename T>
using Rect = Rect_Orthogonal<T>;

export namespace Geom {
    template <typename Cont, Concepts::Number T>
    class QuadTree {
        using obtainer = std::function<const Rect<T>&(const Cont&)>;
    protected:
        obtainer transfer;
        // The boundary of this node
        Rect<T> boundary{};

        Rect<T>& obtainBound(const Cont* const cont) {
            return transfer(*cont);
        }

        Rect<T>& obtainBound(const Cont& cont) {
            return transfer(cont);
        }

        // The rectangles in this node
        std::vector<const Cont*> rectangles{};

        // The four children of this node
        QuadTree* northwest{nullptr};
        QuadTree* northeast{nullptr};
        QuadTree* southwest{nullptr};
        QuadTree* southeast{nullptr};

    public:
        unsigned int maximumItemCount = 4;
        explicit QuadTree(const Rect<T>& boundary, const obtainer& transfer) : transfer(transfer), boundary(boundary) {
            rectangles.reserve(maximumItemCount);
        }

        ~QuadTree() {
            delete northwest;
            delete northeast;
            delete southwest;
            delete southeast;
        }

        [[nodiscard]] Rect<T>& getBoundary() {
            return boundary;
        }

        void setBoundary(const Rect<T>& boundary) {
            this->boundary = boundary;
        }

        void setBoundary(const T x, const T y, const T width, const T height) {
            this->boundary.set(x, y, width, height);
        }

        void setBoundary(const T width, const T height) {
            if(!leaf())throw ext::RuntimeException{"QuadTree Boundary Should be set initially"};

            this->boundary.setSize(width, height);
        }

        // ReSharper disable once CppDFAConstantFunctionResult
        [[nodiscard]] bool overlap(const Cont& box) const {
            // Check if the rectangle overlaps with any rectangle in this node
            if(
                const Rect<T>& rectangle = transfer(box);
                std::any_of(rectangles.begin(), rectangles.end(), [this, &rectangle](const Cont* cont) {
                    return rectangle.overlap(obtainBound(cont));
                })
            ) {
                return true;
            }

            // If this node has children, check if the rectangle overlaps with any rectangle in the children
            if (!leaf()) {
                //TODO uses direction vector to get more possible results?
                if (northwest->overlap(box)) return true;
                if (northeast->overlap(box)) return true;
                if (southwest->overlap(box)) return true;
                if (southeast->overlap(box)) return true;
            }

            // Otherwise, the rectangle does not overlap with any rectangle in the quad tree
            return false;
        }

        void remove(const Cont& box) {
            // Check if the rectangle is in this node
            auto it = std::find(rectangles.begin(), rectangles.end(), box);
            if (it != rectangles.end()) {
                // If the rectangle is in this node, remove it
                rectangles.erase(it);
            }else if (!leaf()) { // If this node has children, check if the rectangle is in the children
                northwest->remove(box);
                northeast->remove(box);
                southwest->remove(box);
                southeast->remove(box);
            }

            // Check if this node should be merged
            if (rectangles.empty() && allChildrenEmpty()) {
                delete northwest;
                delete northeast;
                delete southwest;
                delete southeast;
                northwest = nullptr;
                northeast = nullptr;
                southwest = nullptr;
                southeast = nullptr;
            }
        }

        [[nodiscard]] bool leaf() const {
            return northwest == nullptr || northeast == nullptr || southeast == nullptr || southwest == nullptr;
        }

        [[nodiscard]] bool allChildrenEmpty() const {
            if (!leaf()) {
                if (!northwest->rectangles.empty() || !northeast->rectangles.empty() ||
                    !southwest->rectangles.empty() || !southeast->rectangles.empty()) {
                    return false;
                }
            }

            return true;
        }

        bool insert(const Cont& box) {
            // Ignore objects that do not belong in this quad tree
            if (boundary.dissociated(transfer(box))) {
                return false;
            }

            // If there is space in this quad tree and if it does not subdivide, add the object here
            if (rectangles.size() < maximumItemCount) {
                rectangles.push_back(&box);
                return true;
            }

            // Otherwise, subdivide and then add the object to whichever node will accept it
            if (leaf()) {
                subDivide();
            }

            if (northwest->insert(box)) return true;
            if (northeast->insert(box)) return true;
            if (southwest->insert(box)) return true;
            if (southeast->insert(box)) return true;

            // Otherwise, the object cannot be inserted for some unknown reason (this should never happen)
            return false;
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

            northwest = new QuadTree(nw, transfer);
            northeast = new QuadTree(ne, transfer);
            southwest = new QuadTree(sw, transfer);
            southeast = new QuadTree(se, transfer);
        }
    };
}
