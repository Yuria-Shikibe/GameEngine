export module Math.BinPacker2D;

import <vector>;
import <ranges>;
import <algorithm>;
import Concepts;
import Geom.QuadTree;
export import Geom.Shape.Rect_Orthogonal;
import <numeric>;

/*
*
 */
export namespace Math {
    template <Concepts::Number T>
    Geom::Shape::Rect_Orthogonal<T> pack_rectangles(std::vector<Geom::Shape::Rect_Orthogonal<T>>& rectangles) {
        using Rect = Geom::Shape::Rect_Orthogonal<T>;

        std::sort(rectangles.begin(), rectangles.end(), [](const Rect& a, const Rect& b){
            return a.getHeight() > b.getHeight();
        });

        // Initialize the bounding box
        int width = 0;
        for (const auto& r : rectangles) {
            width += r.getWidth();
        }

        int height = rectangles[0].getHeight();

        // Initialize the quadtree with the bounding box
        const T maxWidth = std::accumulate(rectangles.begin(), rectangles.end(), 0, [](const T val, const Rect& d) {
            return val + d.getWidth();
        });

        const T maxHeight = std::accumulate(rectangles.begin(), rectangles.end(), 0, [](const T val, const Rect& d) {
            return val + d.getHeight();
        });

        Geom::QuadTree<T> tree{{maxWidth, maxHeight}};

        // Place each rectangle
        for (auto& rectangle : rectangles) {
            // Find the best position where the rectangle fits
            int bestX = -1, bestY = -1, bestWaste = width * height;
            for (int x = 0; x <= width - rectangle.getWidth(); ++x) {
                for (int y = 0; y <= height - rectangle.getHeight(); ++y) {
                    // Check if the rectangle overlaps with any placed rectangle
                    if (Rect temp(x, y, rectangle.getWidth(), rectangle.getHeight()); !tree.overlap(temp)) {
                        // Calculate the waste of this position
                        // Update the best position
                        if (
                            const int waste =
                                (width  - x - rectangle.getWidth() ) * rectangle.getHeight() +
                                (height - y - rectangle.getHeight()) * rectangle.getWidth();
                            waste < bestWaste
                        ) {
                            bestX = x;
                            bestY = y;
                            bestWaste = waste;
                        }
                    }
                }
            }
            // Place the rectangle at the best position
            if (bestX != -1 && bestY != -1) {
                rectangle.setSrcX(bestX);
                rectangle.setSrcY(bestY);
                tree.insert(rectangle);
            }
        }

        // Return the size of the bounding box
        return Rect{width, height};
    }
}
