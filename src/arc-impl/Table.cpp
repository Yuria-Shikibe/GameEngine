module UI.Table;

void UI::Table::layoutRelative() {
	if(cells.empty() || std::ranges::all_of(cells, std::identity{}, &LayoutCell::ignore))return;

	auto curLayoutRows =
		std::ranges::count_if(cells, std::identity{}, &LayoutCell::endRow);

	//Make Sure There at least exist one row!
	if(!cells.back().endRow())curLayoutRows++;

	elemLayoutCount.y = static_cast<int>(curLayoutRows);

	{ //Register Row Max width / height
		int curElemPerRow = 0;
		float totalRowMaxHeight = 0.0f;

		Geom::Vec2 currentCellSize{};

		for(const auto& cell : cells) {
			if(!cell.ignore()){
				curElemPerRow++;

				if(!cell.scaleRelativeToParentX) {
					currentCellSize.x += cell.getHoriScale() * cell.item->getWidth();
				}

				if(!cell.scaleRelativeToParentY) {
					currentCellSize.y = std::max(cell.getScaledCellHeight(), currentCellSize.y);
				}
			}

			if(cell.endRow()){
				elemLayoutCount.x = std::max(elemLayoutCount.x, curElemPerRow);
				totalRowMaxHeight += currentCellSize.y;
				bound.setLargerWidth(currentCellSize.x);

				currentCellSize.setZero();
				curElemPerRow = 0;
			}
		}

		//This is a patch if the loop for last linefeed
		elemLayoutCount.x = std::max(elemLayoutCount.x, curElemPerRow);
		totalRowMaxHeight += currentCellSize.y;
		bound.setLargerWidth(currentCellSize.x);

		bound.setLargerHeight(totalRowMaxHeight);
	}

	//Split all into boxes
	//TODO should cells have their own column or row data?
	Geom::Point2 curPos{};

	// [y0, y1, ... yn, x1, x2, ... xn]
	std::vector<float> maxSizeArr(columns() + curLayoutRows);

	{ //Assign Cell Position
		const float spacingX = std::max(0.0f, bound.getWidth() - getBorderWidth())  / static_cast<float>(columns());
		const float spacingY = std::max(0.0f, bound.getHeight() - getBorderHeight()) / static_cast<float>(curLayoutRows);

		for(auto& cell : cells) {
			if(!cell.ignore()){
				cell.allocatedBound.setSize(spacingX, spacingY);

				cell.allocatedBound.setSrc(
					spacingX * static_cast<float>(curPos.x),
					spacingY * static_cast<float>(curLayoutRows - curPos.y - 1) //Top src to Bottom src transform
				);

				cell.applySizeToItem();

				const auto curPosX_indexed = curLayoutRows + curPos.x;

				maxSizeArr[curPosX_indexed] = std::max(maxSizeArr[curPosX_indexed], cell.getCellWidth());
				maxSizeArr[curPos.y] = std::max(maxSizeArr[curPos.y], cell.getCellHeight());

				curPos.x++;
			}

			if(cell.endRow()) {
				curPos.y++;
				curPos.x = 0;
			}
		}
	}

	curPos.setZero();

	{
		Geom::Vec2 currentSrcCoord{
			0.0f,
			std::accumulate(maxSizeArr.begin() + 1, maxSizeArr.begin() + curLayoutRows, 0.0f)
		};

		for(auto& cell : cells) {
			if(!cell.ignore()){
				//TODO fix this for shrink, should uses accumlate
				cell.allocatedBound.setSrc(currentSrcCoord);

				cell.applyPosToItem(this);

				currentSrcCoord.x += maxSizeArr[curLayoutRows + curPos.x];
				curPos.x++;
			}

			if(cell.endRow()) {
				curPos.y++;
				curPos.x = 0;
				currentSrcCoord.x = 0;
				currentSrcCoord.y -= maxSizeArr[curPos.y];
			}
		}
	}

	setSize(
		std::accumulate(maxSizeArr.begin() + curLayoutRows, maxSizeArr.end(), 0.0f) + getBorderWidth(),
		std::accumulate(maxSizeArr.begin(), maxSizeArr.begin() + curLayoutRows, 0.0f) + getBorderHeight()
	);
}

void UI::Table::layoutIrrelative() {
	for(auto& cell : cells) {
		if(cell.ignore())continue;

		cell.allocatedBound = bound;

		// cell.allocatedBound.setSrc(0, 0);
		cell.allocatedBound.setSrc(border.bot_lft());
		cell.allocatedBound.addSize(-getBorderWidth(), -getBorderHeight());

		cell.applySizeToItem();
		cell.applyPosToItem(this);
	}
}
