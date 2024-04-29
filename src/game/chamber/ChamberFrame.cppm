//
// Created by Matrix on 2024/4/18.
//

export module Game.ChamberFrame;

import std;
export import Game.Chamber;
import Geom.QuadTree;
import Concepts;
import ext.RuntimeException;
import Math;
import ext.Algorithm;

export namespace Game{
	/**
	 * @brief This frame should never be dynamic used! since insert or erase a new chamber cost too much time
	 */
	class ChamberFrame{
		[[nodiscard]] static Geom::Rect_Orthogonal<float> getBoundOf(const ChamberTile& ChamberTile){
			return ChamberTile.getChamberRealRegion();
		}

	public:
		using Tile = ChamberTile;
		using TreeType = Geom::QuadTree<Tile, float, &getBoundOf>;

		struct TileBrief{
			std::vector<Game::Chamber*> owners{};
			std::vector<const Game::ChamberTile*> invalids{};
			std::vector<const Game::ChamberTile*> valids{};

			bool dataValid{false};

			constexpr void reset() noexcept{
				dataValid = false;
				owners.clear();
				invalids.clear();
				valids.clear();
			}
		};
		
	protected:
		TileBrief mutable brief;

		Geom::OrthoRectFloat bound{};

		std::list<Tile> data{};
		TreeType quadTree{5};

		//TODO Is all these worth it to sustain this vector?
		//Maybe directly uses umap is a better choise

		std::unordered_map<Geom::Point2, Tile*> positionRef{};

		void eraseRef(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				quadTree.remove(itr->second);
				positionRef.erase(itr);
			}
		}

		bool shouldInsert(const ChamberTile& tile) const {
			if(const auto finded = find(tile.pos)){
				if(!tile.ownsChamber())return false;

				if(!finded->valid()){
					return true;
				}

				throw ext::IllegalArguments{"Insertion Failed: Cannot Insert Two Valid Tile in the same pos!"};
			}
			return true;
		}

		void padReference(Tile& src, const bool insertToMap = true){

			if(src.getChamberRegion().area() <= 1)return;

			src.getChamberRegion().each([this, &src, insertToMap](const Geom::Point2 pos){
				if(pos == src.pos)return;

				const Tile* tile{nullptr};

				if(insertToMap){
					if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
						tile = itr->second;
					}
				}else{
					if(const auto itr = std::ranges::find(data, pos, &Tile::pos); itr != data.end()){
						tile = itr.operator->();
					}
				}

				if(tile){
					if(tile->valid()){
						throw ext::IllegalArguments{"Overlap Valid Should Never Happend During MultiTile Chamber Insertion!"};
					}
					erase(pos);
				}
			});

			src.getChamberRegion().each([this, &src, insertToMap](const Geom::Point2 pos){
				if(pos == src.pos)return;

				if(insertToMap){
					insert(Tile{pos, &src});
				}else{
					data.emplace_back(pos, &src);
				}
			});
		}

		void resetBrief() const{
			if(!brief.dataValid)return;
			brief.~TileBrief();
			new (&brief) TileBrief;
		}


	public:
		ChamberFrame(){
			quadTree.setStrict(false);
		}

		void updateBrief() const{
			if(brief.dataValid)return;

			for (auto& chamberTile : data){
				if(chamberTile.ownsChamber()){
					brief.owners.push_back(chamberTile.chamber.get());
				}

				if(chamberTile.valid()){
					brief.valids.push_back(&chamberTile);
				}else{
					brief.invalids.push_back(&chamberTile);
				}
			}

			brief.dataValid = true;
		}

		[[nodiscard]] const TileBrief& getBrief() const{
			updateBrief();
			return brief;
		}

		[[nodiscard]] TreeType& getQuadTree() noexcept { return quadTree; }

		[[nodiscard]] const std::list<Tile>& getData() const noexcept{ return data; }
		[[nodiscard]] std::list<Tile>& getData() noexcept{ return data; }

		[[nodiscard]] const std::unordered_map<Geom::Point2, Tile*>& getPositionRef() const noexcept{ return positionRef; }
		[[nodiscard]] std::unordered_map<Geom::Point2, Tile*>& getPositionRef() noexcept{ return positionRef; }

		[[nodiscard]] const Tile& at(const Geom::Point2 pos) const{
			return *positionRef.at(pos);
		}

		[[nodiscard]] Tile& at(const Geom::Point2 pos){
			return *positionRef.at(pos);
		}

		[[nodiscard]] const Tile* find(const Geom::Point2 pos) const{
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				return itr->second;
			}
			return nullptr;
		}

		[[nodiscard]] Tile* find(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				return itr->second;
			}
			return nullptr;
		}

		[[nodiscard]] Chamber* findChamber(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				if(itr->second->ownsChamber())return itr->second->chamber.get();
				if(itr->second->referenceTile)return itr->second->referenceTile->chamber.get();
			}
			return nullptr;
		}

		[[nodiscard]] const Chamber* findChamber(const Geom::Point2 pos) const{
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				if(itr->second->ownsChamber())return itr->second->chamber.get();
				if(itr->second->referenceTile)return itr->second->referenceTile->chamber.get();
			}
			return nullptr;
		}

		/**
		 * @brief Be cautionous about iterator failure
		 * @param setToInvalid erase relative tiles or set them to invalid
		 */
		void erase(const Geom::Point2 chamberPos, const bool setToInvalid = false){
			resetBrief();

			if(auto itr = std::ranges::find(data, chamberPos, &Tile::pos); itr != data.end()){
				Tile* chamber = itr.operator->();

				if(chamber->refOnly()){
					itr = std::ranges::find(data, chamber->referenceTile->pos, &Tile::pos);
					if(itr == data.end()){
						throw ext::IllegalArguments{"Failed to find ref tile!"};
					}

					chamber = itr.operator->();
				}


				const decltype(data)::iterator begin = itr;
				auto end = begin;
				std::advance(end, chamber->getChamberRegion().area());

				if(setToInvalid){
					for(auto& element : std::ranges::subrange{begin, end}){
						element.setReference(nullptr);
						element.chamber.reset();
					}
				}else{
					chamber->getChamberRegion().each([this](const Geom::Point2 subPos){
						eraseRef(subPos);
					});

					data.erase(begin, end);
				}
			}
		}

		/**
		 * @brief
		 * @param tile Tile to insert
		 * @param noTreeInsertion whether to insert tile to the quad tree now, used for IO and editor when quad tree is not needed
		 */
		void insert(Tile&& tile, const bool noTreeInsertion = false){
			if(!shouldInsert(tile))return;

			tile.init();
			erase(tile.pos);
			data.push_back(std::move(tile));
			auto& val = data.back();

			positionRef.insert_or_assign(val.pos, &val);

			if(!noTreeInsertion)tryInsertTree(val, true);
			padReference(val, true);

			resetBrief();
		}

		void erase(const Tile* val){
			erase(val->pos);
		}

		template <Concepts::Iterable<Tile> Range>
		void build(Range&& input){
			quadTree.clear();
			positionRef.clear();
			data.clear();
			resetBrief();

			//TODO better bound allocation if the origin point of tilemap doesn't begin at (0, 0) [esp > (0, 0)]
			bound.set(0, 0, 0, 0);

			for(Tile& tile : input){
				if(!shouldInsert(tile))continue;
				erase(tile.pos);
				data.push_back(std::forward<std::ranges::range_value_t<Range>>(tile));
				positionRef.insert_or_assign(tile.pos, &data.back());

				Tile& t = data.back();
				t.init();
				t.getChamberRegion().each([this, &t](const Geom::Point2 pos){
					if(pos == t.pos)return;

					erase(pos);
					data.emplace_back(pos, &t);
					positionRef.insert_or_assign(pos, &data.back());
				});
			}

			reMap(true);
			reTree();
		}

		void reMap(const bool resizeBound = false){
			positionRef.clear();
			quadTree.clearItemsOnly();

			positionRef.reserve(data.size());

			if(data.empty())return;

			if(resizeBound){
				if(Math::zero(bound.area())){
					bound = data.front().getTileBound();
				}

				for (const auto& val : data){
					bound.expandBy(getBoundOf(val));
				}
			}
			
			for (auto& tile : data){
				positionRef.insert_or_assign(tile.pos, &tile);
			}
		}

		void reTree(){
			quadTree.clearItemsOnly();
			quadTree.setBoundary(bound);

			for (auto& tile : data){
				tryInsertTree(tile);
			}
		}

		void tryInsertTree(Tile& val, const bool boundCheck = false){
			// if(val.ownsChamber()){
				quadTree.insert(&val);

				if(boundCheck){
					const auto oriBound = bound;

					bound.expandBy(getBoundOf(val));
					if(bound != oriBound){
						quadTree.clear();
						quadTree.setBoundary(bound);
						for(auto& bound : data/* | std::ranges::views::filter(&ChamberTile::ownsChamber)*/){
							quadTree.insert(&bound);
						}
					}
				}
			// }
		}

		[[nodiscard]] bool contains(const Geom::Point2 point2) const {
			return positionRef.contains(point2);
		}

		[[nodiscard]] bool placementValid(const Geom::OrthoRectInt region) const {
			for(int x = region.getSrcX(); x < region.getEndX(); ++x){
				for(int y = region.getSrcY(); y < region.getEndY(); ++y){
					if(const auto tile = positionRef.find({x, y}); tile != positionRef.end()){
						if(tile->second->valid())return false;
					}else{
						return false;
					}
				}
			}

			return true;
		}


		/** @return [compliant not] */
		auto getPartedTiles(Concepts::Invokable<bool(const ChamberTile&)> auto&& pred) const{
			return ext::partBy(data, pred);
		}

		/** @return [compliant not] */
		auto getPartedTiles(Concepts::Invokable<bool(const ChamberTile&)> auto&& pred){
			return ext::partBy(data, pred);
		}

		[[nodiscard]] Geom::Point2 getNearbyPos(Geom::Vec2 pos) const{
			return pos.div(TileSize).trac<int>();
		}

		[[nodiscard]] const Geom::OrthoRectFloat& getBound() const{ return bound; }
		
		[[nodiscard]] Geom::OrthoRectInt getTiledBound() const{
			Geom::OrthoRectFloat boundSize = bound;
			boundSize.sclSize(1 / TileSize, 1 / TileSize);
			return boundSize.round<int>();
		}
	};
}


export
	template <>
	struct ::Core::IO::JsonSerializator<Game::ChamberFrame>{
		static constexpr std::string_view InvalidTiles = "it";
		static constexpr std::string_view OwnerTiles = "ot";
		static void write(ext::json::JsonValue& jsonValue, const Game::ChamberFrame& data){
			auto& map = jsonValue.asObject();

			ext::json::JsonValue owners{};
			ext::json::JsonValue invalids{};
			auto& ownersArray = owners.asArray();
			auto& invalidsArray = invalids.asArray();

			ownersArray.reserve(data.getData().size() / 3);
			invalidsArray.reserve(data.getData().size() / 2);

			for (const auto & chamberTile : data.getData()){
				if(chamberTile.ownsChamber()){
					ownersArray.push_back(ext::json::getJsonOf(chamberTile));
					continue;
				}

				if(!chamberTile.valid()){
					invalidsArray.push_back(ext::json::getJsonOf(chamberTile.pos));
				}
			}

			map.insert_or_assign(OwnerTiles, std::move(owners));
			map.insert_or_assign(InvalidTiles, std::move(invalids));
		}

		static void read(const ext::json::JsonValue& jsonValue, Game::ChamberFrame& data){
			auto& map = jsonValue.asObject();
			auto& owners = map.at(OwnerTiles).asArray();

			for (const auto & jval : owners){
				auto tile = ext::json::getValueFrom<Game::ChamberTile>(jval);
				data.insert(std::move(tile), false);
			}

			auto& invalids = map.at(InvalidTiles).asArray();

			for (const auto & jval : invalids){
				data.insert(Game::ChamberTile{ext::json::getValueFrom<Geom::Point2>(jval)}, false);
			}

			data.reTree();
		}
	};