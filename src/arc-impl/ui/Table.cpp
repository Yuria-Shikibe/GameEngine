module UI.Table;

import Math;


//TODO this is a temp import
import Graphic.Draw;

void UI::Table::layoutRelative(){
	if(cells.empty() || std::ranges::all_of(cells, std::identity{}, &LayoutCell::isIgnoreLayout)) return;

	recalculateLayoutSize();

	cells.back().item->setEndRow(true);

	// [y0, y1, ... yn, x1, x2, ... xn]
	std::vector<std::pair<float, bool>> maxSizeArr(columns() + rows());
	// maxSizeArr.resize();

	//Split all into boxes
	//TODO should cells have their own column or row data?


	Geom::Point2 scaleRequester{};

	bool expandX{false};
	bool expandY{false};

	bool expandX_ifLarger{true};
	bool expandY_ifLarger{true};

	bool hasRatioCalculate{false};

	{
		//Register Self Adapted Row Max width & height
		Geom::Point2 currentLineScaleRequester{};

		for(Geom::Point2 curPos{}; const auto& cell : cells){
			if(cell.isIgnoreLayout()) continue;

			hasRatioCalculate = hasRatioCalculate || cell.hasRatioFromHeight() || cell.hasRatioFromWidth();

			if(!cell.scaleRelativeToParentX){
				const auto curPosX_indexed = rows() + curPos.x;
				maxSizeArr[curPosX_indexed] = {Math::max(maxSizeArr[curPosX_indexed].first, cell.getDefWidth()), true};
			} else{
				currentLineScaleRequester.x++;
			}

			if(!cell.scaleRelativeToParentY){
				maxSizeArr[curPos.y] = {Math::max(maxSizeArr[curPos.y].first, cell.getDefHeight()), true};
			} else{
				currentLineScaleRequester.y = 1;
			}

			expandX |= cell.modifyParentX;
			expandY |= cell.modifyParentY;

			expandX_ifLarger &= cell.modifyParentX_ifLarger;
			expandY_ifLarger &= cell.modifyParentY_ifLarger;

			curPos.x++;

			if(cell.isEndRow()){
				curPos.y++;
				curPos.x = 0;

				scaleRequester.maxX(currentLineScaleRequester.x);
				scaleRequester.y += currentLineScaleRequester.y;

				currentLineScaleRequester.setZero();
			}
		}
	}

	{

	allocateBound:{
			//Assign Cell Position
			float capturedW{0};
			float capturedH{0};

			const int remainColomns{scaleRequester.x};
			const int remainRows{scaleRequester.y};

			auto captured = maxSizeArr | std::ranges::views::elements<1>;

			capturedW = std::accumulate(captured.begin() + rows(), captured.end(), 0);
			capturedH = std::accumulate(captured.begin(), captured.begin() + rows(), 0);

			for(int y = 0; y < rows(); ++y){
				if(maxSizeArr[y].second) capturedH += maxSizeArr[y].first;
			}

			for(int x = rows(); x < maxSizeArr.size(); ++x){
				if(maxSizeArr[x].second) capturedW += maxSizeArr[x].first;
			}

			const float spacingX = remainColomns
				                       ? Math::clampPositive(
					                       (getValidWidth() - capturedW) / static_cast<float>(remainColomns))
				                       : 0.0f;
			const float spacingY = remainRows
				                       ? Math::clampPositive(
					                       (getValidHeight() - capturedH) / static_cast<float>(remainRows))
				                       : 0.0f;

			for(int y = 0; y < rows(); ++y){
				if(!maxSizeArr[y].second) maxSizeArr[y].first = spacingY;
			}

			for(int x = rows(); x < maxSizeArr.size(); ++x){
				if(!maxSizeArr[x].second) maxSizeArr[x].first = spacingX;
			}
		}

		if(hasRatioCalculate){
			hasRatioCalculate = false;

			for(Geom::Point2 curPos{}; const auto& cell : cells){
				if(cell.isIgnoreLayout()) continue;

				if(cell.hasRatioFromHeight()){
					const auto curPosX_indexed = rows() + curPos.x;
					maxSizeArr[curPosX_indexed].first = Math::max(maxSizeArr[curPosX_indexed].first,
						maxSizeArr[curPos.y].first * cell.getRatio_H2W());
					maxSizeArr[curPosX_indexed].second |= maxSizeArr[curPos.y].second;
				}


				if(cell.hasRatioFromWidth()){
					const auto curPosX_indexed = rows() + curPos.x;
					maxSizeArr[curPos.y].first = Math::max(maxSizeArr[curPos.y].first,
						maxSizeArr[curPosX_indexed].first * cell.getRatio_W2H());
					maxSizeArr[curPos.y].second |= maxSizeArr[curPosX_indexed].second;
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
				if(!scaleRequester.x || Math::zero(getWidth())) setWidth(cellSize.x + getBorderWidth());
			}
		}

		if(expandY && !fillParentY){
			if(expandY_ifLarger){
				setHeight(Math::max(cellSize.y + getBorderHeight(), getHeight()));
			} else{
				if(!scaleRequester.y || Math::zero(getHeight())) setHeight(cellSize.y + getBorderHeight());
			}
		}

		//TODO cells may expand during this process
		//Need another re-layout to handle this
		{
			Geom::Vec2 currentSrc{};
			Geom::Vec2 currentPad{};
			Geom::Vec2 maximumPad{};

			Geom::Vec2 currentMaxSize{};

			cellSize.setZero();

			for(Geom::Point2 curPos{}; auto& cell : cells){
				if(cell.isIgnoreLayout()) continue;

				const auto curPosX_indexed = rows() + curPos.x;

				cell.allocatedBound.setSize(maxSizeArr[curPosX_indexed].first, maxSizeArr[curPos.y].first);

				cell.allocatedBound.setSrc(
					currentSrc.x, currentSrc.y - cell.allocatedBound.getHeight() //Top src to Bottom src transform
				);

				const Geom::Vec2 thisPad{cell.pad.left, -cell.pad.top};

				cell.applySizeToItem();
				cell.allocatedBound.move(Geom::Vec2{currentPad.x, -maximumPad.y} + thisPad);

				curPos.x++;
				currentSrc.x += maxSizeArr[curPosX_indexed].first;

				currentPad.x += cell.getPadHori();
				currentPad.maxY(cell.getPadVert());

				currentMaxSize.x += cell.getCellWidth();
				currentMaxSize.maxY(cell.getCellHeight());

				if(cell.isEndRow()){
					maximumPad.maxX(currentPad.x);
					maximumPad.y += currentPad.y;

					cellSize.maxX(currentMaxSize.x);
					cellSize.y += currentMaxSize.y;

					currentPad.setZero();
					currentMaxSize.setZero();

					currentSrc.x = 0;
					currentSrc.y -= maxSizeArr[curPos.y].first;

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

	if(relativeLayoutFormat){
		layoutRelative();
	} else{
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
