module UI.Table;

import Math;

void UI::Table::layoutRelative() {
	if(cells.empty() || std::ranges::all_of(cells, std::identity{}, &LayoutCell::isIgnoreLayout))return;

	recalculateLayoutSize();

	// [y0, y1, ... yn, x1, x2, ... xn]
	std::vector<float> maxSizeArr(columns() + rows());

	auto getYOffset = [&maxSizeArr, row = rows()]{
		return std::accumulate(maxSizeArr.begin() + 1, maxSizeArr.begin() + row, 0.0f);
	};

	//Split all into boxes
	//TODO should cells have their own column or row data?
	Geom::Point2 curPos{};

	bool expandX{false};
	bool expandY{false};

	{ //Register Row Max width / height
		float totalRowMaxHeight = 0.0f;

		Geom::Vec2 currentCellSize{};

		for(const auto& cell : cells) {
			if(!cell.isIgnoreLayout()){
				if(!cell.scaleRelativeToParentX) {
					float cellWidth = cell.getExpectedItemWidth();
					currentCellSize.x += cellWidth;
					const auto curPosX_indexed = rows() + curPos.x;
					maxSizeArr[curPosX_indexed] = std::max(maxSizeArr[curPosX_indexed], cellWidth);
				}

				if(!cell.scaleRelativeToParentY) {
					currentCellSize.maxY(cell.getExpectedItemHeight());
					maxSizeArr[curPos.y] = std::max(maxSizeArr[curPos.y], currentCellSize.y);
				}

				expandX |= cell.modifyParentX;
				expandY |= cell.modifyParentY;

				curPos.x++;
			}

			if(cell.isEndRow()){
				totalRowMaxHeight += currentCellSize.y;
				if(expandX)bound.setLargerWidth(currentCellSize.x);

				currentCellSize.setZero();

				curPos.y++;
				curPos.x= 0;
			}
		}

		if(!cells.back().isEndRow()){
			//This is a patch if the loop for last linefeed
			totalRowMaxHeight += currentCellSize.y;
			if(expandX)bound.setLargerWidth(currentCellSize.x);
		}

		curPos.setZero();
		if(expandY)bound.setLargerHeight(totalRowMaxHeight);
	}

	{ //Assign Cell Position
		float captureW{0};
		float captureH{0};

		int remainColomns{this->columns()};
		int remainRows{this->rows()};
		for(int y = 0; y < rows(); ++y){
			const float h = maxSizeArr[y];
			if(Math::zero(h))continue;
			captureH += h;
			remainRows--;
		}

		for(int x = rows(); x < maxSizeArr.size(); ++x){
			const float w = maxSizeArr[x];
			if(Math::zero(w))continue;
			captureW += w;
			remainColomns--;
		}

		const float spacingX = remainColomns ? std::max(0.0f, (this->getValidWidth() - captureW) / static_cast<float>(remainColomns)) : 0.0f;
		const float spacingY = remainRows ? std::max(0.0f, (this->getValidHeight() - captureH) / static_cast<float>(remainRows)) : 0.0f;

		for(int y = 0; y < rows(); ++y){
			maxSizeArr[y] = std::max(maxSizeArr[y], spacingY);
		}

		for(int x = rows(); x < maxSizeArr.size(); ++x){
			maxSizeArr[x] = std::max(maxSizeArr[x], spacingX);
		}

		float currentSpacingX = 0;
		float currentSpacingY = getYOffset();

		for(auto& cell : cells) {
			if(!cell.isIgnoreLayout()){
				const auto curPosX_indexed = rows() + curPos.x;

				cell.allocatedBound.setSize(maxSizeArr[curPosX_indexed], maxSizeArr[curPos.y]);

				cell.allocatedBound.setSrc(
					currentSpacingX,
					currentSpacingY //Top src to Bottom src transform
				);

				cell.applySizeToItem();
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

	if(expandX){
		setWidth(std::accumulate(maxSizeArr.begin() + rows(), maxSizeArr.end(), 0.0f) + getBorderWidth());
	}

	if(expandY){
		setHeight(std::accumulate(maxSizeArr.begin(), maxSizeArr.begin() + rows(), 0.0f) + getBorderHeight());
	}
}

void UI::Table::layoutIrrelative() {
	for(auto& cell : cells) {
		if(cell.isIgnoreLayout())continue;

		cell.allocatedBound = bound;

		// cell.allocatedBound.setSrc(0, 0);
		cell.allocatedBound.setSrc(border.bot_lft());
		cell.allocatedBound.addSize(-getBorderWidth(), -getBorderHeight());

		cell.applySizeToItem();
		cell.applyPosToItem(this);
	}
}
