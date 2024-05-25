//
// Created by Matrix on 2024/4/18.
//

export module Game.Chamber.Frame;

import std;
export import Game.Chamber;
import Geom.QuadTree;
import ext.Concepts;
import ext.RuntimeException;
import Math;
import ext.Algorithm;

namespace Game{
	export
	[[nodiscard]] Geom::Point2 toChamberPos(Geom::Vec2 pos) noexcept{
		return pos.div(TileSize).trac<int>();
	}

	export
	template <typename Entity>
	class ChamberFrameData{
	public:
		// using Entity = Geom::Vec2;
		using EntityType = Entity;
		using Tile = ChamberTile<Entity>;
		using ItrType = typename std::list<Tile>::iterator;

		struct TileBrief{
			std::vector<Game::Chamber<EntityType>*> owners{};
			std::vector<const Tile*> invalids{};
			std::vector<const Tile*> valids{};

			bool dataValid{false};

			constexpr void reset() noexcept{
				dataValid = false;
				owners.clear();
				invalids.clear();
				valids.clear();
			}
		};

		mutable TileBrief brief{};

		void resetBrief() const noexcept{
			if(!brief.dataValid)return;
			brief.reset();
		}

	protected:
		[[nodiscard]] static Geom::OrthoRectFloat getBoundOf(const ChamberTile<Entity>& ChamberTile){
			return ChamberTile.getChamberRealRegion();
		}

		Geom::OrthoRectFloat bound{};
		std::list<Tile> data{};

		//TODO Is all these worth it to sustain this vector?
		//Maybe directly uses umap is a better choise

		void eraseRef(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				positionRef.erase(itr);
			}
		}

		std::unordered_map<Geom::Point2, ItrType> positionRef{};

		bool shouldInsert(const Tile& tile) const {
			if(const auto finded = find(tile.pos)){
				if(!tile.ownsChamber())return false;

				if(!finded->valid()){
					return true;
				}

				throw ext::IllegalArguments{"Insertion Failed: Cannot Insert Two Valid Tile in the same pos!"};
			}
			return true;
		}

		void updateReference(){
			for(Tile& tile : data | std::ranges::views::filter(&Tile::ownsChamber)){
				tile.getChamberRegion().each([this, &tile](const Geom::Point2 pos){
					if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
						itr->second->referenceTile = &tile;
					}
				});
			}
		}

		void padReference(Tile& src){
			if(src.getChamberRegion().area() <= 1)return;

			src.getChamberRegion().each([this, &src](const Geom::Point2 pos){
				if(pos == src.pos)return;

				const Tile* tile{nullptr};

				auto itr = findItrByPos(pos);
				if(itr != data.end())tile = itr.operator->();

				if(tile){
					if(tile->valid()){
						throw ext::IllegalArguments{"Overlap Valid Should Never Happend During MultiTile Chamber Insertion!"};
					}
					erase(pos);
				}
			});

			src.getChamberRegion().each([this, &src](const Geom::Point2 pos){
				if(pos == src.pos)return;

				this->insert(Tile{pos, &src});
			});
		}

		auto findItrByPos(const Geom::Point2 pos){
			auto itr = positionRef.find(pos);

			if(itr == positionRef.end()){
				return std::ranges::find(data, pos, &Tile::pos);
			}else{
				return itr->second;
			}
		}
	public:
		[[nodiscard]] ChamberFrameData() = default;

		ChamberFrameData(const ChamberFrameData& other){
			this->build(std::list<Tile>{other.data} | std::ranges::views::filter(std::not_fn(&Tile::refOnly)));
		}

		ChamberFrameData(ChamberFrameData&& other) noexcept
			: bound{std::move(other.bound)},
			  data{std::move(other.data)},
			  positionRef{std::move(other.positionRef)}{}

		ChamberFrameData& operator=(const ChamberFrameData& other){
			if(this == &other) return *this;
			this->build(std::list<Tile>{other.data} | std::ranges::views::filter(std::not_fn(&Tile::refOnly)));
			return *this;
		}

		ChamberFrameData& operator=(ChamberFrameData&& other) noexcept{
			if(this == &other) return *this;
			bound = std::move(other.bound);
			data = std::move(other.data);
			positionRef = std::move(other.positionRef);
			return *this;
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

		[[nodiscard]] bool isBriefValid() const noexcept{
			return brief.dataValid;
		}

		[[nodiscard]] const std::list<Tile>& getData() const noexcept{ return data; }
		[[nodiscard]] std::list<Tile>& getData() noexcept{ return data; }

		[[nodiscard]] auto& getPositionRef() const noexcept{ return positionRef; }
		[[nodiscard]] auto& getPositionRef() noexcept{ return positionRef; }

		[[nodiscard]] const Tile& at(const Geom::Point2 pos) const{
			return positionRef.at(pos).operator*();
		}

		[[nodiscard]] Tile& at(const Geom::Point2 pos){
			return positionRef.at(pos).operator*();
		}

		[[nodiscard]] const Tile* find(const Geom::Point2 pos) const{
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				return itr->second.operator->();
			}
			return nullptr;
		}

		[[nodiscard]] Tile* find(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				return itr->second.operator->();
			}
			return nullptr;
		}

		[[nodiscard]] ItrType findItr(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				return itr->second;
			}
			return data.end();
		}

		[[nodiscard]] Chamber<EntityType>* findChamber(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				if(itr->second->ownsChamber())return itr->second->chamber.get();
				if(itr->second->referenceTile)return itr->second->referenceTile->chamber.get();
			}
			return nullptr;
		}

		[[nodiscard]] const Chamber<EntityType>* findChamber(const Geom::Point2 pos) const{
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

			//TODO optm this
			auto itr = findItrByPos(chamberPos);

			if(itr != data.end()){
				const Tile* chamber = itr.operator->();

				if(chamber->refOnly()){
					itr = findItrByPos(chamber->referenceTile->pos);
					if(itr == data.end()){
						throw ext::IllegalArguments{"Failed to find ref tile!"};
					}

					chamber = itr.operator->();
				}


				const typename decltype(data)::iterator begin = itr;
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
		 */
		void insert(Tile&& tile){
			if(!this->shouldInsert(tile))return;

			resetBrief();

			tile.init();
			this->erase(tile.pos);
			data.push_back(std::move(tile));
			const auto& itr = std::prev(data.end());

			positionRef.insert_or_assign(itr->pos, ItrType{itr});
			this->padReference(itr.operator*());
		}

		void erase(const Tile* val){
			this->erase(val->pos);
		}

		template <Concepts::Iterable<Tile> Range>
		void build(Range&& input){
			positionRef.clear();
			data.clear();

			//TODO better bound allocation if the origin point of tilemap doesn't begin at (0, 0) [esp > (0, 0)]
			bound.set(0, 0, 0, 0);

			for(Tile& tile : input){
				if(!shouldInsert(tile))continue;
				erase(tile.pos);
				data.push_back(std::forward<std::ranges::range_value_t<Range>>(tile));
				positionRef.insert_or_assign(tile.pos, std::prev(data.end()));

				Tile& t = data.back();
				t.init();
				t.getChamberRegion().each([this, &t](const Geom::Point2 pos){
					if(pos == t.pos)return;

					erase(pos);
					data.emplace_back(pos, &t);
					positionRef.insert_or_assign(pos, std::prev(data.end()));
				});
			}

			reMap(true);
		}

		void resizeBound(){
			if(data.empty()){
				bound = {};
				return;
			}

			bound = data.front().getTileBound();
			for (const auto& val : data){
				bound.expandBy(this->getBoundOf(val));
			}
		}

		void reMap(const bool resizeBound = false){
			positionRef.clear();

			positionRef.reserve(data.size());

			if(data.empty())return;

			if(resizeBound){
				this->resizeBound();
			}

			for(auto cur = data.begin(); cur != data.end(); ++cur){
				positionRef.insert_or_assign(cur->pos, cur);
			}

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


		/** @return [compliant, not] */
		auto getPartedTiles(Concepts::Invokable<bool(const Tile&)> auto&& pred) const{
			return ext::partBy(data, pred);
		}

		/** @return [compliant, not] */
		auto getPartedTiles(Concepts::Invokable<bool(const Tile&)> auto&& pred){
			return ext::partBy(data, pred);
		}

		[[nodiscard]] const Geom::OrthoRectFloat& getBound() const{ return bound; }

		[[nodiscard]] Geom::OrthoRectInt getTiledBound() const{
			Geom::OrthoRectFloat boundSize = bound;
			boundSize.sclSize(1 / TileSize, 1 / TileSize);
			return boundSize.round<int>();
		}

		struct JsonSrl{
			static constexpr std::string_view InvalidTiles = "it";
			static constexpr std::string_view OwnerTiles = "ot";

			static void write(ext::json::JsonValue& jsonValue, const ChamberFrameData& data){
				auto& map = jsonValue.asObject();

				ext::json::JsonValue owners{};
				ext::json::JsonValue invalids{};
				auto& ownersArray = owners.asArray();
				auto& invalidsArray = invalids.asArray();

				ownersArray.reserve(data.getData().size() / 3);
				invalidsArray.reserve(data.getData().size() / 2);

				for(const auto& chamberTile : data.getData()){
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

			static void read(const ext::json::JsonValue& jsonValue, ChamberFrameData& data){
				auto& map = jsonValue.asObject();
				auto& owners = map.at(OwnerTiles).asArray();

				for(const auto& jval : owners){
					auto tile = ext::json::getValueFrom<Tile>(jval);
					data.insert(std::move(tile));
				}

				auto& invalids = map.at(InvalidTiles).asArray();

				for(const auto& jval : invalids){
					data.insert(Tile{ext::json::getValueFrom<Geom::Point2>(jval)});
				}

				data.resizeBound();
			}
		};
	};
	/**
	 * @brief This frame should never be dynamic used! since insert or erase a new chamber cost too much time
	 */
	export
	template <typename Entity>
	class ChamberFrame : public ChamberFrameData<Entity>{
	public:
		using ChamberFrameData<Entity>::ChamberFrameData;
		using typename ChamberFrameData<Entity>::Tile;
		using typename ChamberFrameData<Entity>::EntityType;
		using ChamberFrameData<Entity>::positionRef;
		using ChamberFrameData<Entity>::data;
		using ChamberFrameData<Entity>::bound;

		using TreeType = Geom::QuadTree<Tile, float, &ChamberFrame::getBoundOf>;

	protected:
		using ChamberFrameData<Entity>::eraseRef;
		using ChamberFrameData<Entity>::padReference;
		using ChamberFrameData<Entity>::shouldInsert;

		TreeType quadTree{5};

	public:
		ChamberFrame(){
			quadTree.setStrict(false);
		}

		[[nodiscard]] TreeType& getQuadTree() noexcept { return quadTree; }


		/**
		 * @brief
		 * @param tile Tile to insert
		 * @param noTreeInsertion whether to insert tile to the quad tree now, used for IO and editor when quad tree is not needed
		 */
		void insert(Tile&& tile, const bool noTreeInsertion = false){
			ChamberFrameData<Entity>::insert(std::move(tile));

			auto& val = data.back();
			if(!noTreeInsertion)tryInsertTree(val, true);
		}

		template <Concepts::Iterable<Tile> Range>
		void build(Range&& input){
			quadTree.clear();
			this->resetBrief();

			ChamberFrameData<Entity>::build(std::move(input));

			reTree();
		}

		void reMap(const bool resizeBound = false){
			quadTree.clearItemsOnly();
			ChamberFrameData<Entity>::reMap(resizeBound);
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

					bound.expandBy(this->getBoundOf(val));
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
	};
}


