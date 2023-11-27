module;

export module Geom.QuadTree;

export import Geom.Shape.Rect_Orthogonal;

import <vector>;
import Concepts;

using Geom::Shape::Rect_Orthogonal;

template <typename T>
using Rect = Rect_Orthogonal<T>;

export namespace Geom {
    template <Concepts::Number T>
    class QuadTree {
    protected:
        // The boundary of this node
        Rect<T> boundary{};

        // The rectangles in this node
        std::vector<Rect<T>> rectangles{};

        // The four children of this node
        QuadTree* northwest{nullptr};
        QuadTree* northeast{nullptr};
        QuadTree* southwest{nullptr};
        QuadTree* southeast{nullptr};

    public:
        explicit QuadTree(const Rect<T>& boundary) : boundary(boundary) {}

        ~QuadTree() {
            delete northwest;
            delete northeast;
            delete southwest;
            delete southeast;
        }

        bool overlap(const Rect_Orthogonal<T>& rectangle) const {
            // Check if the rectangle overlaps with any rectangle in this node
            for (const auto& r : rectangles) {
                if (rectangle.overlap(r)) {
                    return true;
                }
            }

            // If this node has children, check if the rectangle overlaps with any rectangle in the children
            if (!leaf()) {
                if (northwest->overlap(rectangle)) return true;
                if (northeast->overlap(rectangle)) return true;
                if (southwest->overlap(rectangle)) return true;
                if (southeast->overlap(rectangle)) return true;
            }

            // Otherwise, the rectangle does not overlap with any rectangle in the quad tree
            return false;
        }

        void remove(const Rect_Orthogonal<T>& rectangle) {
            // Check if the rectangle is in this node
            auto it = std::find(rectangles.begin(), rectangles.end(), rectangle);
            if (it != rectangles.end()) {
                // If the rectangle is in this node, remove it
                rectangles.erase(it);
            }

            // If this node has children, check if the rectangle is in the children
            if (!leaf()) {
                northwest->remove(rectangle);
                northeast->remove(rectangle);
                southwest->remove(rectangle);
                southeast->remove(rectangle);
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
            return northwest == nullptr;
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

        bool insert(const Rect<T>& rectangle) {
            // Ignore objects that do not belong in this quad tree
            if (!boundary.overlap(rectangle)) {
                return false;
            }

            // If there is space in this quad tree and if it does not subdivide, add the object here
            if (rectangles.size() < 4) {
                rectangles.push_back(rectangle);
                return true;
            }

            // Otherwise, subdivide and then add the object to whichever node will accept it
            if (leaf()) {
                subDivide();
            }

            if (northwest->insert(rectangle)) return true;
            if (northeast->insert(rectangle)) return true;
            if (southwest->insert(rectangle)) return true;
            if (southeast->insert(rectangle)) return true;

            // Otherwise, the object cannot be inserted for some unknown reason (this should never happen)
            return false;
        }

        void subDivide() {
            T x = boundary.getSrcX();
            T y = boundary.getSrcY();
            T w = boundary.getWidth() / static_cast<T>(2);
            T h = boundary.getHeight() / static_cast<T>(2);

            Rect<T> nw(x    , y    , w, h);
            Rect<T> ne(x + w, y    , w, h);
            Rect<T> sw(x    , y + h, w, h);
            Rect<T> se(x + w, y + h, w, h);

            northwest = new QuadTree(nw);
            northeast = new QuadTree(ne);
            southwest = new QuadTree(sw);
            southeast = new QuadTree(se);
        }
    };
}
