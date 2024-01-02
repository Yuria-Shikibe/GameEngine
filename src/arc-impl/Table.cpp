module UI.Table;

void UI::Table::layoutRelative() {
	if(cells.empty() || std::ranges::all_of(std::as_const(cells), [](const LayoutCell& cell) {
		return cell.ignore();
	}))return;

	size_t curLayoutRows = std::ranges::count_if(std::as_const(cells), [](const auto& t) {
		return t.endRow();
	});

	//Make Sure There at least exist one row!
	if(!cells.back().endRow()) {
		curLayoutRows++;
	}

	rowsCount = curLayoutRows;

	//Register Row Max width / height

	{
		size_t curElemPerRow        = 0;
		float currentColunmMaxWidth = 0.0f;

		float currentRowMaxHeight = 0.0f;
		float totalRowMaxHeight   = 0.0f;

		for(const auto& cell : cells) {
			if(!cell.scaleRelativeToParentX) {
				currentColunmMaxWidth += cell.widthScale() * cell.item->getWidth();
			}

			if(cell.ignore()) {
				if(cell.endRow())goto endRow;
			}

			curElemPerRow++;

			if(!cell.scaleRelativeToParentY) {
				currentRowMaxHeight = std::fmaxf(cell.heightScale() * cell.item->getHeight(), currentRowMaxHeight);
			}

			if(cell.endRow())goto endRow;

			continue;

			endRow:
				totalRowMaxHeight += currentRowMaxHeight;
			bound.setLargerWidth(currentColunmMaxWidth);
			currentRowMaxHeight = currentColunmMaxWidth = 0.0f;

			if(curElemPerRow > maxElemPerRow) {
				maxElemPerRow = curElemPerRow;
			}

			curElemPerRow = 0;
		}

		if(curElemPerRow > maxElemPerRow) {
			maxElemPerRow = curElemPerRow;
		}

		totalRowMaxHeight += currentRowMaxHeight;

		bound.setLargerWidth(currentColunmMaxWidth);
		bound.setLargerHeight(totalRowMaxHeight);
	}

	//Split all into boxes
	//TODO should cells have their own column or row data?
	int curX{0};
	int curY{0};

	//TODO is this necessary?
	std::vector<float> maxSizeArr(columns() + curLayoutRows);


	{
		const float spacingX = std::fmaxf(0.0f, bound.getWidth() - marginWidth())  / static_cast<float>(columns());
		const float spacingY = std::fmaxf(0.0f, bound.getHeight() - marginHeight()) / static_cast<float>(curLayoutRows);

		for(auto& cell : cells) {
			if(cell.ignore()) {
				if(cell.endRow()) {
					curY++;
					curX = 0;
				}
				continue;
			}

			cell.allocatedBound.setSize(spacingX, spacingY);

			cell.allocatedBound.setSrc(spacingX * curX, spacingY * (curLayoutRows - curY - 1));

			cell.applySize();

			maxSizeArr[curLayoutRows + curX] = std::fmaxf(maxSizeArr[curLayoutRows + curX], cell.getCellWidth());
			maxSizeArr[curY]                 = std::fmaxf(maxSizeArr[curY], cell.getCellHeight());

			curX++;
			if(cell.endRow()) {
				curY++;
				curX = 0;
			}
		}
	}

	curX = curY = 0;

	{
		float currentSrcX = 0;
		float currentSrcY = std::accumulate(maxSizeArr.begin() + 1, maxSizeArr.begin() + curLayoutRows, 0.0f);

		for(auto& cell : cells) {
			if(cell.ignore()) {
				if(cell.endRow()) {
					curY++;
					curX = 0;
				}
				continue;
			}

			//TODO fix this for shrink, should uses accumlate
			cell.allocatedBound.setSrc(
				currentSrcX,
				currentSrcY
			);

			cell.applyPos(this);

			currentSrcX += maxSizeArr[curLayoutRows + curX];

			curX++;
			if(cell.endRow()) {
				curY++;
				curX = 0;
				currentSrcX = 0;
				currentSrcY -= maxSizeArr[curY];
			}
		}
	}

	setSize(
		std::accumulate(maxSizeArr.begin() + curLayoutRows, maxSizeArr.end(), 0.0f) + marginWidth(),
		std::accumulate(maxSizeArr.begin(), maxSizeArr.begin() + curLayoutRows, 0.0f) + marginHeight()
	);
}

void UI::Table::layoutIrrelative() {
	for(auto& cell : cells) {
		if(cell.ignore())continue;

		cell.allocatedBound = bound;

		// cell.allocatedBound.setSrc(0, 0);
		cell.allocatedBound.setSrc(margin_bottomLeft.x, margin_bottomLeft.y);
		cell.allocatedBound.addSize(-marginWidth(), -marginHeight());

		cell.applySize();
		cell.applyPos(this);
	}
}
