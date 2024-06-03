module UI.Table;

import Math;


//TODO this is a temp import
import Graphic.Draw;

void UI::Table::layoutRelative() {
	if(cells.empty() || std::ranges::all_of(cells, std::identity{}, &LayoutCell::isIgnoreLayout))return;

	recalculateLayoutSize();

	cells.back().item->setEndRow(true);

	// [y0, y1, ... yn, x1, x2, ... xn]
	std::basic_string<float> maxSizeArr{};
	maxSizeArr.resize(columns() + rows());

	//Split all into boxes
	//TODO should cells have their own column or row data?
	Geom::Point2 curPos{};

	Geom::Point2 scaleRequester{};

	bool expandX{false};
	bool expandY{false};

	bool expandX_ifLarger{true};
	bool expandY_ifLarger{true};

	{ //Register Self Adapted Row Max width & height
		Geom::Point2 currentLineScaleRequester{};

		for(const auto& cell : cells) {
			if(cell.isIgnoreLayout())continue;

			if(!cell.scaleRelativeToParentX) {
				const auto curPosX_indexed = rows() + curPos.x;
				maxSizeArr[curPosX_indexed] = Math::max(maxSizeArr[curPosX_indexed], cell.getDefWidth());
			}else{
				currentLineScaleRequester.x++;
			}

			if(!cell.scaleRelativeToParentY) {
				maxSizeArr[curPos.y] = Math::max(maxSizeArr[curPos.y], cell.getDefHeight());
			}else{
				currentLineScaleRequester.y = 1;
			}

			expandX |= cell.modifyParentX;
			expandY |= cell.modifyParentY;

			expandX_ifLarger &= cell.modifyParentX_ifLarger;
			expandY_ifLarger &= cell.modifyParentY_ifLarger;

			curPos.x++;

			if(cell.isEndRow()){
				curPos.y++;
				curPos.x= 0;

				scaleRequester.maxX(currentLineScaleRequester.x);
				scaleRequester.y += currentLineScaleRequester.y;

				currentLineScaleRequester.setZero();
			}
		}

		curPos.setZero();
	}

	{
		//Assign Cell Position
		float capturedW{0};
		float capturedH{0};

		const int remainColomns{scaleRequester.x};
		const int remainRows{scaleRequester.y};

		for(int y = 0; y < rows(); ++y){
			const float h = maxSizeArr[y];
			if(Math::zero(h)){
				maxSizeArr[y] -= 1.0f;
				continue;
			}
			capturedH += h;
		}

		for(int x = rows(); x < maxSizeArr.size(); ++x){
			const float w = maxSizeArr[x];
			if(Math::zero(w)){
				maxSizeArr[x] -= 1.0f;
				continue;
			}
			capturedW += w;
		}

		const float spacingX = remainColomns ? Math::max(0.0f, (getValidWidth() - capturedW) / static_cast<float>(remainColomns)) : 0.0f;
		const float spacingY = remainRows ? Math::max(0.0f, (getValidHeight() - capturedH) / static_cast<float>(remainRows)) : 0.0f;

		for(int y = 0; y < rows(); ++y){
			if(maxSizeArr[y] < 0)maxSizeArr[y] = spacingY;
		}

		for(int x = rows(); x < maxSizeArr.size(); ++x){
			if(maxSizeArr[x] < 0)maxSizeArr[x] = spacingX;
		}

		Geom::Vec2 cellSize{
				std::accumulate(maxSizeArr.begin() + rows(), maxSizeArr.end(), 0.0f),
				std::accumulate(maxSizeArr.begin(), maxSizeArr.begin() + rows(), 0.0f)
			};

		if(expandX && !fillParentX){
			if(expandX_ifLarger){
				setWidth(Math::max(cellSize.x + getBorderWidth(), getWidth()));
			}else{
				if(!scaleRequester.x || Math::zero(getWidth()))setWidth(cellSize.x + getBorderWidth());
			}
		}

		if(expandY && !fillParentY){
			if(expandY_ifLarger){
				setHeight(Math::max(cellSize.y + getBorderHeight(), getHeight()));
			}else{
				if(!scaleRequester.y || Math::zero(getHeight()))setHeight(cellSize.y + getBorderHeight());
			}
		}

		//TODO cells may expand during this process
		//Need another re-layout to handle this
		Geom::Vec2 currentSrc{};
		Geom::Vec2 currentPad{};
		Geom::Vec2 maximumPad{};
		
		Geom::Vec2 currentMaxSize{};

		cellSize.setZero();

		for(auto& cell : cells) {
			if(cell.isIgnoreLayout())continue;

			const auto curPosX_indexed = rows() + curPos.x;

			cell.allocatedBound.setSize(maxSizeArr[curPosX_indexed], maxSizeArr[curPos.y]);

			cell.allocatedBound.setSrc(
				currentSrc.x, currentSrc.y - cell.allocatedBound.getHeight() //Top src to Bottom src transform
			);

			const Geom::Vec2 thisPad{cell.pad.left, -cell.pad.top};

			cell.applySizeToItem();
			cell.allocatedBound.move(Geom::Vec2{currentPad.x, -maximumPad.y} + thisPad);

			curPos.x++;
			currentSrc.x += maxSizeArr[curPosX_indexed];

			currentPad.x += cell.getPadHori();
			currentPad.maxY(cell.getPadVert());

			currentMaxSize.x += cell.getCellWidth();
			currentMaxSize.maxY(cell.getCellHeight());

			if(cell.isEndRow()) {
				maximumPad.maxX(currentPad.x);
				maximumPad.y += currentPad.y;

				cellSize.maxX(currentMaxSize.x);
				cellSize.y += currentMaxSize.y;

				currentPad.setZero();
				currentMaxSize.setZero();

				currentSrc.x = 0;
				currentSrc.y -= maxSizeArr[curPos.y];

				curPos.y++;
				curPos.x = 0;
			}
		}

		if(!maximumPad.isZero()){
			cellSize += maximumPad;

			if(expandX && !fillParentX){
				if(expandX_ifLarger){
					setWidth(Math::max(cellSize.x + getBorderWidth(), getWidth()));
				}else{
					if(!scaleRequester.x)setWidth(cellSize.x + getBorderWidth());
				}
			}

			if(expandY && !fillParentY){
				if(expandY_ifLarger){
					setHeight(Math::max(cellSize.y + getBorderHeight(), getHeight()));
				}else{
					if(!scaleRequester.y)setHeight(cellSize.y + getBorderHeight());
				}
			}
		}

		Geom::Vec2 offset = Align::getOffsetOf(cellAlignMode, cellSize, getValidBound());
		offset.y += cellSize.y;

		for(auto& cell : cells) {
			if(cell.isIgnoreLayout())continue;

			cell.allocatedBound.move(offset);
			cell.applyPosToItem(this);
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

void UI::Table::layout(){
	layout_tryFillParent();

	layoutChildren();

	if(relativeLayoutFormat) {
		layoutRelative();
	}else {
		layoutIrrelative();
	}

	lastSignal = lastSignal - ChangeSignal::notifySubs;
	Group::layout();
}

void UI::Table::drawContent() const{
	// for(auto& cell : cells){
	// 	Rect rect{cell.allocatedBound};
	// 	rect.move(absoluteSrc);
	//
	// 	using namespace Graphic;
	// 	Draw::color(Colors::YELLOW);
	// 	Draw::Line::setLineStroke(1.0f);
	// 	Draw::Line::rectOrtho(rect);
	// 	Draw::Line::line(absoluteSrc, absoluteSrc + border.bot_lft());
	// }

	Group::drawContent();

}
