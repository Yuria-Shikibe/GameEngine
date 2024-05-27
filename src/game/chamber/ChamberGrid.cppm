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
// using Entity = int;
	export
	template <typename Entity>
	class ChamberGridData{
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
			return ChamberTile.getChamberRegion();
		}

		Geom::OrthoRectFloat bound{};
		std::list<Tile> data{};
		std::unordered_map<Geom::Point2, ItrType> positionRef{};

		//TODO Is all these worth it to sustain this vector?
		//Maybe directly uses umap is a better choise


		void eraseData(const Geom::Point2 pos){
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				data.erase(itr->second);
				positionRef.erase(itr);
			}
		}

		void buildRef(){
			for(auto cur = data.begin(); cur != data.end(); ++cur){
				positionRef.try_emplace(cur->pos, cur);
			}
		}

		bool checkOverlap(const Tile& tile) const {
			if(const auto finded = this->find(tile.pos)){
				//Invalid tile should not replace the valid one
				if(!tile.isOwner())return false;

				//Valid tile can replace the invalid one
				if(!finded->valid()){
					return true;
				}

				//Place valid to valid should pop a exception
				throw ext::IllegalArguments{"Insertion Failed: Cannot Insert Two Valid Tile in the same pos!"};
			}
			return true;
		}

		void appendBack(const Tile& tile){
			data.push_back(tile);
			positionRef.insert_or_assign(tile.pos, std::prev(data.end()));
		}

		void insertOrAssign(const Tile& tile){
			auto itr = findItrByPos(tile.pos);
			if(itr != data.end()){
				itr->chamber = tile.chamber;
			}else{
				this->appendBack(tile);
			}
		}
		/**
		 * @brief Should always be state like this after pad:
		 * [..., owner, ref[1], ref[2], ..., ref[owner.area() - 1], ...]
		 * So owner should be the first of it's ref tiles
		 *
		 * @warning Notice that only owners should run this function
		 *
		 * @return true if src is owner and insert happens
		 */
		bool ownerInsert(const Tile& src){
			if(!src.isOwner())return false;

			//Clear the original tiles in the bound
			src.getChamberGridBound().each([this](const Geom::Point2 pos){
#if DEBUG_CHECK
				const Tile* tile{nullptr};

				auto itr = findItrByPos(pos);
				if(itr != data.end())tile = itr.operator->();

				if(tile){
					if(tile->valid()){
						throw ext::IllegalArguments{"Overlap Valid Should Never Happend During MultiTile Chamber Insertion!"};
					}
					erase(pos);
				}
#else
				erase(pos);
#endif
			});

			//Insert new tiles
			src.getChamberGridBound().each([this, chamber = src.chamber](const Geom::Point2 pos){
				this->appendBack(Tile{pos, chamber});
			});

			return true;
		}

		ItrType findItrByPos(const Geom::Point2 pos){
			const auto itr = positionRef.find(pos);

			if(itr == positionRef.end()){
				return std::ranges::find(data, pos, &Tile::pos);
			}else{
				return itr->second;
			}
		}
	public:
		[[nodiscard]] ChamberGridData() = default;


		ChamberGridData(ChamberGridData&& other) noexcept
			: bound{other.bound}, data{std::move(other.data)}{
			buildRef();
		}

		ChamberGridData& operator=(ChamberGridData&& other) noexcept{
			if(this == &other) return *this;
			data = std::move(other.data);
			bound = other.bound;

			buildRef();
			return *this;
		}

		ChamberGridData(const ChamberGridData& other) : bound{other.bound}, data{other.data}{
			buildRef();
		}

		ChamberGridData& operator=(const ChamberGridData& other){
			if(this == &other) return *this;
			data = other.data;
			bound = other.bound;

			buildRef();

			return *this;
		}

		void updateBrief() const{
			if(brief.dataValid)return;

			for (auto& chamberTile : data){
				if(chamberTile.isOwner()){
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
				return itr->second->chamber.get();
			}
			return nullptr;
		}

		[[nodiscard]] const Chamber<EntityType>* findChamber(const Geom::Point2 pos) const{
			if(const auto itr = positionRef.find(pos); itr != positionRef.end()){
				return itr->second->chamber.get();
			}
			return nullptr;
		}

		/**
		 * @brief Be cautionous about iterator failure
		 * @param setToInvalid erase relative tiles or set them to invalid
		 */
		void erase(const Geom::Point2 chamberPos, const bool setToInvalid = false){
			resetBrief();

			if(auto itr = findItrByPos(chamberPos); itr != data.end()){
				const Tile* chamber = itr.operator->();

				if(chamber->valid()){
					itr = this->findItrByPos(chamber->getPosOfRef());
					if(itr == data.end()){
						throw ext::IllegalArguments{"Failed to find ref tile!"};
					}

					chamber = itr.operator->();
				}

				const typename decltype(data)::iterator begin = itr;
				auto end = begin;

				std::advance(end, chamber->getChamberGridBound().area());

				if(setToInvalid){
					for(auto& [pos, chamberPtr] : std::ranges::subrange{begin, end}){
						chamberPtr.reset();
					}
				}else{
					chamber->getChamberGridBound().each([this](const Geom::Point2 subPos){
						positionRef.erase(subPos);
					});

					data.erase(begin, end);
				}
			}
		}

		/**
		 * @brief
		 * @param tile Tile to insert
		 */
		void insert(const Tile& tile){
			if(!this->checkOverlap(tile))return;

			resetBrief();
			bound.expandBy(this->getBoundOf(tile));

			if(this->ownerInsert(tile))return;

			auto itr = this->findItrByPos(tile.pos);

			this->erase(tile.pos);
			this->appendBack(tile);
		}

		void erase(const Tile* val){
			this->erase(val->pos);
		}

		template <Concepts::Iterable<Tile> Range>
		void build(Range&& input){
			if(std::ranges::empty(input))return;

			resetBrief();

			const Tile& front = std::ranges::begin(input).operator*();
			positionRef.clear();
			data.clear();

			//TODO better bound allocation if the origin point of tilemap doesn't begin at (0, 0) [esp > (0, 0)]
			bound = getBoundOf(front);

			for(Tile& tile : input){
				insertOrAssign(tile);
			}
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

			static void write(ext::json::JsonValue& jsonValue, const ChamberGridData& data){
				auto& map = jsonValue.asObject();

				ext::json::JsonValue owners{};
				ext::json::JsonValue invalids{};
				auto& ownersArray = owners.asArray();
				auto& invalidsArray = invalids.asArray();

				ownersArray.reserve(data.getData().size() / 3);
				invalidsArray.reserve(data.getData().size() / 2);

				for(const auto& chamberTile : data.getData()){
					if(chamberTile.isOwner()){
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

			static void read(const ext::json::JsonValue& jsonValue, ChamberGridData& data){
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
	class ChamberGrid : public ChamberGridData<Entity>{
	public:
		using ChamberGridData<Entity>::ChamberGridData;
		using typename ChamberGridData<Entity>::Tile;
		using typename ChamberGridData<Entity>::EntityType;
		using ChamberGridData<Entity>::positionRef;
		using ChamberGridData<Entity>::data;
		using ChamberGridData<Entity>::bound;

		using TreeType = Geom::QuadTree<Tile, float, &ChamberGrid::getBoundOf>;

	protected:

		TreeType quadTree{5};

	public:
		ChamberGrid(){
			quadTree.setStrict(false);
		}

		[[nodiscard]] TreeType& getQuadTree() noexcept { return quadTree; }


		/**
		 * @brief
		 * @param tile Tile to insert
		 * @param noTreeInsertion whether to insert tile to the quad tree now, used for IO and editor when quad tree is not needed
		 */
		void insert(const Tile& tile, const bool noTreeInsertion = false){
			auto cur = data.end();

			ChamberGridData<Entity>::insert(tile);

			if(!noTreeInsertion)for(auto& val : data
				| std::ranges::views::reverse
				| std::ranges::views::take(tile.getChamberGridBound().area())){
				this->tryInsertTree(val, true);
			}
		}

		template <Concepts::Iterable<Tile> Range>
		void build(Range&& input){
			quadTree.clear();

			ChamberGridData<Entity>::build(std::move(input));

			reTree();
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


