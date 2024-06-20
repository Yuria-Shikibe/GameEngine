module UI.Table;

import Math;


//TODO this is a temp import
import Graphic.Draw;

void UI::Table::layoutRelative(){
	if(cells.empty() || std::ranges::all_of(cells, std::identity{}, &LayoutCell::isIgnoreLayout)) return;

	recalculateLayoutSize();

	cells.back().endLine();

	enum Priority : int{
		soft, //No Explicit Size Reuqire
		soft_ratio, //Ratioed Size From Implicit Size
		hard_ratio, //Ratioed Size From Explicit Size
		hard //Explicit Size
	};

	// [y0, y1, ... yn, x1, x2, ... xn]
	std::vector<std::pair<float, Priority>> maxSizeArr(columns() + rows());
	// maxSizeArr.resize();

	//Split all into boxes
	//TODO should cells have their own column or row data?

	Geom::Vec2 maximumPad{};

	Geom::Point2 scaleRequester{};

	bool expandX{false};
	bool expandY{false};

	bool expandX_ifLarger{true};
	bool expandY_ifLarger{true};

	bool hasRatioCalculate{false};

	{
		//Register Self Adapted Row Max width & height
		Geom::Point2 currentLineScaleRequester{};
		Geom::Vec2 currentPad{};

		for(Geom::Point2 curPos{}; const auto& cell : cells){
			if(cell.isIgnoreLayout()) continue;

			hasRatioCalculate = hasRatioCalculate || cell.hasRatioFromHeight() || cell.hasRatioFromWidth();

			if(cell.scaleRelativeToParentX){
				currentLineScaleRequester.x++;
			} else if(cell.isSlaveX()){

			} else if(!cell.hasRatioFromHeight()){
				auto& [size, priority] = maxSizeArr[rows() + curPos.x];
				size = Math::max(size, cell.getDefWidth() + cell.getMarginHori());
				priority = Math::max(priority, hard);
			}

			if(cell.scaleRelativeToParentY){
				currentLineScaleRequester.y = 1;
			} else if(cell.isSlaveY()){

			} else if(!cell.hasRatioFromWidth()){
				auto& [size, priority] = maxSizeArr[curPos.y];
				size = Math::max(size, cell.getDefHeight() + cell.getMarginVert());
				priority = Math::max(priority, hard);
			}


			expandX |= cell.modifyParentX;
			expandY |= cell.modifyParentY;

			expandX_ifLarger &= cell.modifyParentX_ifLarger;
			expandY_ifLarger &= cell.modifyParentY_ifLarger;

			currentPad.x += cell.getPadHori();
			currentPad.maxY(cell.getPadVert());

			curPos.x++;

			if(cell.isEndRow()){
				curPos.y++;
				curPos.x = 0;

				maximumPad.maxX(currentPad.x);
				maximumPad.y += currentPad.y;

				scaleRequester.maxX(currentLineScaleRequester.x);
				scaleRequester.y += currentLineScaleRequester.y;

				currentLineScaleRequester.setZero();
				currentPad.setZero();
			}
		}
	}

	allocateBound:{
		//Assign Cell Position
		Geom::Vec2 capturedSize{};
		Geom::Vec2 slaveRatioCapturedSize{};

		Geom::Point2 masterCells{};
		Geom::Point2 slaveRatioCells{};

		static constexpr auto isMaster = [](const Priority priority) constexpr {return priority >= hard_ratio;};

		{
			auto cur = maxSizeArr.begin();
			for(; cur < maxSizeArr.begin() + rows(); ++cur){
				auto& [size, priority] = *cur;
				if(priority == soft_ratio){
					slaveRatioCapturedSize.y += size;
					slaveRatioCells.y++;
				}else if(isMaster(priority)){
					capturedSize.y += size;
					masterCells.y++;
				}
			}

			for(; cur < maxSizeArr.end(); ++cur){
				auto& [size, priority] = *cur;

				if(priority == soft_ratio){
					slaveRatioCapturedSize.x += size;
					slaveRatioCells.x++;
				}else if(isMaster(priority)){
					capturedSize.x += size;
					masterCells.x++;
				}
			}
		}

		Geom::Vec2 validSpace{getValidSize() - capturedSize - slaveRatioCapturedSize - maximumPad};

		Geom::Point2 slaveCells = scaleRequester - slaveRatioCells;

		if(slaveCells.x)validSpace.x /= static_cast<float>(slaveCells.x);
		else validSpace.x = 0;

		if(slaveCells.y)validSpace.y /= static_cast<float>(slaveCells.y);
		else validSpace.y = 0;

		validSpace.maxX(0);
		validSpace.maxY(0);

		{
			auto cur = maxSizeArr.begin();
			for(; cur < maxSizeArr.begin() + rows(); ++cur){
				auto& [size, priority] = *cur;
				if(priority == soft)size = validSpace.y;
			}

			for(; cur < maxSizeArr.end(); ++cur){
				auto& [size, priority] = *cur;
				if(priority == soft)size = validSpace.x;
			}
		}
	}

	if(hasRatioCalculate){
		hasRatioCalculate = false;

		for(Geom::Point2 curPos{}; const auto& cell : cells){
			if(cell.isIgnoreLayout()) continue;

			const auto curPosX_indexed = rows() + curPos.x;
			auto& sizeDataX = maxSizeArr[curPosX_indexed];
			auto& sizeDataY = maxSizeArr[curPos.y];

			if(cell.hasRatioFromHeight() && sizeDataX.second != hard){
				sizeDataX.first = Math::max(sizeDataX.first, sizeDataY.first * cell.getRatio_H2W());

				switch(sizeDataY.second){
					case hard : sizeDataX.second = Math::max(hard_ratio, sizeDataX.second); break;
					case soft_ratio: [[fallthrough]];
					case soft : sizeDataX.second = Math::max(soft_ratio, sizeDataX.second); break;
					default: break;
				}
			}


			if(cell.hasRatioFromWidth() && sizeDataY.second != hard){
				sizeDataY.first = Math::max(sizeDataY.first, sizeDataX.first * cell.getRatio_W2H());

				switch(sizeDataX.second){
					case hard : sizeDataY.second = Math::max(hard_ratio, sizeDataY.second); break;
					case soft_ratio: [[fallthrough]];
					case soft : sizeDataY.second = Math::max(soft_ratio, sizeDataY.second); break;
					default: break;
				}
			}

			curPos.x++;

			if(cell.isEndRow()){
				curPos.y++;
				curPos.x = 0;
			}
		}

		goto allocateBound;
	}

	auto sizes = maxSizeArr | std::ranges::views::elements<0>;
	Geom::Vec2 cellSize{
			std::accumulate(sizes.begin() + rows(), sizes.end(), 0.0f),
			std::accumulate(sizes.begin(), sizes.begin() + rows(), 0.0f)
		};

	if(expandX && !fillParentX){
		if(expandX_ifLarger){
			setWidth(Math::max(cellSize.x + getBorderWidth(), getWidth()));
		} else{
			if(!scaleRequester.x || Math::zero(getValidWidth()) || getValidWidth() > cellSize.x + getBorderWidth()) setWidth(cellSize.x + getBorderWidth());
		}
	}

	if(expandY && !fillParentY){
		if(expandY_ifLarger){
			setHeight(Math::max(cellSize.y + getBorderHeight(), getHeight()));
		} else{
			if(!scaleRequester.y || Math::zero(getValidHeight()) || getValidHeight() > cellSize.y + getBorderHeight()) setHeight(cellSize.y + getBorderHeight());
		}
	}

	//TODO cells may expand during this process
	//Need another re-layout to handle this
	{
		Geom::Vec2 currentSrc{};

		Geom::Vec2 currentMaxSize{};

		cellSize.setZero();

		float maxYPad{};

		for(Geom::Point2 curPos{}; auto& cell : cells){
			if(cell.isIgnoreLayout()) continue;

			const auto curPosX_indexed = rows() + curPos.x;

			cell.allocatedBound.setSize(maxSizeArr[curPosX_indexed].first, maxSizeArr[curPos.y].first);

			cell.allocatedBound.setSrc(
				currentSrc.x + cell.pad.left, currentSrc.y - cell.allocatedBound.getHeight() - cell.pad.top//Top src to Bottom src transform
			);

			cell.applySizeToItem();

			curPos.x++;
			currentSrc.x += maxSizeArr[curPosX_indexed].first + cell.getPadHori();

			maxYPad = Math::max(maxYPad, cell.getPadVert());

			currentMaxSize.x += cell.getCellWidth();
			currentMaxSize.maxY(cell.getCellHeight());

			if(cell.isEndRow()){
				cellSize.maxX(currentMaxSize.x);
				cellSize.y += currentMaxSize.y;

				currentMaxSize.setZero();

				currentSrc.x = 0;
				currentSrc.y -= (maxSizeArr[curPos.y].first + maxYPad);
				maxYPad = 0;

				curPos.y++;
				curPos.x = 0;
			}
		}


		if(!maximumPad.isZero()){
			cellSize += maximumPad;

			if(expandX && !fillParentX){
				if(expandX_ifLarger){
					setWidth(Math::max(cellSize.x + getBorderWidth(), getWidth()));
				} else{
					if(!scaleRequester.x) setWidth(cellSize.x + getBorderWidth());
				}
			}

			if(expandY && !fillParentY){
				if(expandY_ifLarger){
					setHeight(Math::max(cellSize.y + getBorderHeight(), getHeight()));
				} else{
					if(!scaleRequester.y) setHeight(cellSize.y + getBorderHeight());
				}
			}
		}
	}

	Geom::Vec2 offset = Align::getOffsetOf(cellAlignMode, cellSize, getValidBound());
	offset.y += cellSize.y;

	for(auto& cell : cells){
		if(cell.isIgnoreLayout()) continue;

		cell.allocatedBound.move(offset);
		cell.applyPosToItem(this);
	}
}

void UI::Table::layoutIrrelative(){
	for(auto& cell : cells){
		if(cell.isIgnoreLayout()) continue;

		cell.allocatedBound = getValidBound();

		cell.applySizeToItem();
		cell.applyPosToItem(this);
	}
}

void UI::Table::layout(){
	layout_tryFillParent();

	Group::layout();

	if(relativeLayoutFormat){
		layoutRelative();
	} else{
		layoutIrrelative();
	}

	// layoutChildren();
	//
	// layoutChanged = false;
	//
	// if(relativeLayoutFormat){
	// 	layoutRelative();
	// } else{
	// 	layoutIrrelative();
	// }

	Group::layout();

	//
	// Group::layout();
	lastSignal = lastSignal - ChangeSignal::notifySubs;
}

void UI::Table::drawContent() const{
	// for(auto& cell : cells){
	// 	Rect rect{cell.allocatedBound};
	// 	rect.move(absoluteSrc);
	//
	// 	Rect itemBound{cell.item->getBound().setSrc(cell.item->getAbsSrc())};
	//
	// 	using namespace Graphic;
	// 	Draw::Overlay::color(Colors::YELLOW, 0.7f);
	// 	Draw::Overlay::Line::setLineStroke(1.0f);
	// 	Draw::Overlay::Line::rectOrtho(rect);
	//
	// 	Draw::Overlay::color(Colors::ORANGE, 0.67f);
	// 	Draw::Overlay::Line::rectOrtho(itemBound);
	//
	// 	// Draw::Overlay::Line::line(absoluteSrc, cell.item->getAbsSrc());
	//
	// 	if(!rect.containsLoose(itemBound)){
	// 		Draw::Overlay::color(Colors::RED_DUSK, 0.37f);
	// 		Draw::Overlay::Fill::rectOrtho(Draw::Overlay::getContextTexture(), itemBound);
	// 	}
	// }

	Group::drawContent();
}
