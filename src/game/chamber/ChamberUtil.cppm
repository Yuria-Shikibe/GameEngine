//
// Created by Matrix on 2024/4/21.
//

export module Game.Chamber.Util;

export import Graphic.Pixmap;
export import Game.Chamber;
export import Game.ChamberFrame;
import Geom.Rect_Orthogonal;
import Graphic.Color;
import std;
import ext.DynamicBuffer;
import ext.Json;

namespace Colors = Graphic::Colors;

export namespace Game::ChamberUtil{
	using RefType = unsigned;
	constexpr RefType NoChamberDataIndex = std::numeric_limits<RefType>::max();
	using ChamberColorRef = Graphic::Color;

	constexpr ChamberColorRef nullRef = Colors::CLEAR;
	constexpr ChamberColorRef invalid = Colors::BLACK;
	constexpr ChamberColorRef placed = Colors::WHITE;

	[[nodiscard]] ChamberTile parseColor(const ChamberColorRef& color, Geom::Point2 pos){
		return ChamberFactory::genEmptyTile(pos);
	}

	[[nodiscard]] Graphic::Pixmap saveToPixmap(const ChamberFrame& frame){
		const Geom::OrthoRectInt bound = frame.getTiledBound();

		Graphic::Pixmap map{bound.getWidth(), bound.getHeight()};

		for (const auto & tile : frame.getData()){
			Geom::Point2 pixmapPos = tile.pos - bound.getSrc();
			map.set(pixmapPos.x, pixmapPos.y, tile.valid() ? placed : invalid);
		}

		return map;
	}

	[[nodiscard]] ChamberFrame genFrame(const Graphic::Pixmap& map, const float unitLength = 50.0f, const Geom::Point2 offset = {}){
		ChamberFrame frame{};

		map.each([&frame, offset](const Graphic::Pixmap& pixmap, const int x, const int y){
			frame.insert(parseColor(pixmap.get(x, y), offset + Geom::Point2{x, y}));
		});

		return frame;
	}

	class ChamberFrameIO{
		struct PostTask{
			ChamberTile* toFinish{};
			RefType chamberDataIndex{};
			Geom::Point2 refTileOffset{};

			[[nodiscard]] bool hasRef() const {
				return refTileOffset.isZero();
			}
			[[nodiscard]] bool hasData() const {
				return chamberDataIndex != NoChamberDataIndex;
			}
		};

		RefType currentChamberIndex{};

		std::vector<PostTask> postTasks{};
		std::vector<std::vector<char>> dataChunk{};

		std::vector<char> mainDataChunk{};
		ext::DynamicBuffer<> buffer{mainDataChunk};

		std::iostream sideChunkIO;
		std::iostream mainChunkIO;

		void reset(){
			currentChamberIndex = 0;
			postTasks.clear();
			mainDataChunk.clear();
			sideChunkIO.rdbuf(&buffer);
		}

		void writeTile(const ChamberTile& tile){
			mainChunkIO.write(reinterpret_cast<const char*>(&tile.pos), sizeof(tile.pos));

			const auto offset = tile.getOffsetToRef();
			mainChunkIO.write(reinterpret_cast<const char*>(&offset), sizeof(offset));

			if(tile.ownsChamber()){
				mainChunkIO.write(reinterpret_cast<const char*>(&currentChamberIndex), sizeof(currentChamberIndex));
				currentChamberIndex++;
				ext::DynamicBuffer buffer{dataChunk.emplace_back()};
				sideChunkIO.rdbuf(&buffer);
				tile.chamber->write(sideChunkIO);
			}else{
				mainChunkIO.write(reinterpret_cast<const char*>(&NoChamberDataIndex), sizeof(NoChamberDataIndex));
			}
		}

		void readTile(ChamberFrame& frame){
			ChamberTile tile{};

			mainChunkIO.read(reinterpret_cast<char*>(&tile.pos), sizeof(tile.pos));

			PostTask task{};
			mainChunkIO.read(reinterpret_cast<char*>(&task.refTileOffset), sizeof(task.refTileOffset));
			mainChunkIO.read(reinterpret_cast<char*>(&task.chamberDataIndex), sizeof(task.chamberDataIndex));

			frame.getData().push_back(std::move(tile));

			if(task.hasRef() || task.hasData()){
				task.toFinish = &frame.getData().back();
				postTasks.push_back(std::move(task));
			}
		}

	public:
		void writeTo(ext::json::JsonValue& jsonValue, const ChamberFrame& frame){
			reset();

			const Geom::OrthoRectInt bound = frame.getTiledBound();
			const std::size_t size = frame.getData().size();
			mainChunkIO.write(reinterpret_cast<const char*>(&bound), sizeof(bound));
			mainChunkIO.write(reinterpret_cast<const char*>(&size), sizeof(size));

			for (const auto & tile : frame.getData()){
				writeTile(tile);
			}

			const std::size_t chunkSize = dataChunk.size();
			mainChunkIO.write(reinterpret_cast<const char*>(&chunkSize), sizeof(chunkSize));
		}

		void readFrom(std::istream& istream, ChamberFrame& frame){
			reset();
			Geom::OrthoRectInt bound{};
			std::size_t size{};

			istream.read(reinterpret_cast<char*>(&bound), sizeof(bound));
			istream.read(reinterpret_cast<char*>(&size), sizeof(size));

			for(int i = 0; i < size; ++i){
				readTile(frame);
			}

			std::size_t chunkSize{};
			istream.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
			dataChunk.resize(chunkSize);

			for (auto& chunk : dataChunk){

			}

			//TODO async & parallel
			for (auto& task : postTasks){
				if(task.hasRef()){
					task.toFinish->referenceTile = frame.find(task.toFinish->pos + task.refTileOffset);
				}

				if(task.hasData()){
					//TODO datafetch
					task.toFinish->chamber = mockChamberFactory.genChamber();
				}
			}
		}
	};
}
