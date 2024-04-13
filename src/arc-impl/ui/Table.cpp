module UI.Table;

import Math;


//TODO this is a temp import
import Graphic.Draw;

void UI::Table::layoutRelative() {
	if(cells.empty() || std::ranges::all_of(cells, std::identity{}, &LayoutCell::isIgnoreLayout))return;

	recalculateLayoutSize();

	// [y0, y1, ... yn, x1, x2, ... xn]
	std::vector<float> maxSizeArr(columns() + rows());

	//Split all into boxes
	//TODO should cells have their own column or row data?
	Geom::Point2 curPos{};

	bool expandX{false};
	bool expandY{false};

	bool expandX_ifLarger{true};
	bool expandY_ifLarger{true};

	{ //Register Self Adapted Row Max width & height
		for(const auto& cell : cells) {
			if(!cell.isIgnoreLayout()){
				if(!cell.scaleRelativeToParentX) {
					const float cellWidth = cell.getExpectedItemWidth();
					const auto curPosX_indexed = rows() + curPos.x;
					maxSizeArr[curPosX_indexed] = Math::max(maxSizeArr[curPosX_indexed], cellWidth);
				}

				if(!cell.scaleRelativeToParentY) {
					maxSizeArr[curPos.y] = Math::max(maxSizeArr[curPos.y], cell.getExpectedItemHeight());
				}

				expandX |= cell.modifyParentX;
				expandY |= cell.modifyParentY;

				expandX_ifLarger &= cell.modifyParentX_ifLarger;
				expandY_ifLarger &= cell.modifyParentY_ifLarger;

				curPos.x++;
			}

			if(cell.isEndRow()){
				curPos.y++;
				curPos.x= 0;
			}
		}

		curPos.setZero();
	}

	{
		//Assign Cell Position
		float capturedW{0};
		float capturedH{0};

		int remainColomns{columns()};
		int remainRows{rows()};

		for(int y = 0; y < rows(); ++y){
			const float h = maxSizeArr[y];
			if(Math::zero(h))continue;
			capturedH += h;
			remainRows--;
		}

		for(int x = rows(); x < maxSizeArr.size(); ++x){
			const float w = maxSizeArr[x];
			if(Math::zero(w))continue;
			capturedW += w;
			remainColomns--;
		}

		const float spacingX = remainColomns ? Math::max(0.0f, (getValidWidth() - capturedW) / static_cast<float>(remainColomns)) : 0.0f;
		const float spacingY = remainRows ? Math::max(0.0f, (getValidHeight() - capturedH) / static_cast<float>(remainRows)) : 0.0f;

		for(int y = 0; y < rows(); ++y){
			maxSizeArr[y] = Math::max(maxSizeArr[y], spacingY);
		}

		for(int x = rows(); x < maxSizeArr.size(); ++x){
			maxSizeArr[x] = Math::max(maxSizeArr[x], spacingX);
		}

		const float cellWidth = std::accumulate(maxSizeArr.begin() + rows(), maxSizeArr.end(), 0.0f);
		const float cellHeight = std::accumulate(maxSizeArr.begin(), maxSizeArr.begin() + rows(), 0.0f);

		if(expandX){
			if(expandX_ifLarger){
				setWidth(Math::max(cellWidth + getBorderWidth(), getWidth()));
			}else{
				setWidth(cellWidth + getBorderWidth());
			}
		}

		if(expandY){
			if(expandY_ifLarger){
				setHeight(Math::max(cellHeight + getBorderHeight(), getHeight()));
			}else{
				setHeight(cellHeight + getBorderHeight());
			}
		}

		float currentSpacingX = 0;
		float currentSpacingY = std::accumulate(maxSizeArr.begin() + 1, maxSizeArr.begin() + rows(), 0.0f);

		Geom::Vec2 offset = Align::getOffsetOf(cellAlignMode, Geom::Vec2{cellWidth, cellHeight}, getValidBound());

		//TODO cells may expand during this process
		//Need another re-layout to handle this

		bool cellExpandedX{false};
		bool cellExpandedY{false};

		for(auto& cell : cells) {
			if(!cell.isIgnoreLayout()){
				const auto curPosX_indexed = rows() + curPos.x;

				cell.allocatedBound.setSize(maxSizeArr[curPosX_indexed], maxSizeArr[curPos.y]);

				cell.allocatedBound.setSrc(
					currentSpacingX,
					currentSpacingY //Top src to Bottom src transform
				);

				cell.applySizeToItem();
				cell.allocatedBound.move(offset);
				cell.applyPosToItem(this);

				curPos.x++;
				currentSpacingX += maxSizeArr[curPosX_indexed];
			}

			if(cell.isEndRow()) {
				curPos.y++;
				curPos.x = 0;

				currentSpacingX = 0;
				currentSpacingY -= maxSizeArr[curPos.y];
			}
		}


	}
}

void UI::Table::layoutIrrelative() {
	for(auto& cell : cells) {
		if(cell.isIgnoreLayout())continue;

		cell.allocatedBound = getValidBound();

		cell.applySizeToItem();
		cell.applyPosToItem(this);
	}
}

void UI::Table::drawContent() const{
	for(auto& cell : cells){
		Rect rect{cell.allocatedBound};
		rect.move(absoluteSrc);

		using namespace Graphic;
		Draw::color(Colors::YELLOW);
		Draw::Line::setLineStroke(2.0f);
		Draw::Line::rectOrtho(rect);
	}

	Group::drawContent();

}
