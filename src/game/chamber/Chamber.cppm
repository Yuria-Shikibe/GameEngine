//
// Created by Matrix on 2024/4/18.
//

// ReSharper disable CppDFAUnreachableCode
export module Game.Chamber;

export import Geom.Rect_Orthogonal;
export import Geom.Vector2D;
export import Geom.QuadTree.Interface;
import ext.Concepts;
import ext.Owner;
import ext.RuntimeException;
import std;

export import Core.IO.Specialized;

namespace UI{
	export class Table;
}


using Geom::Point2;
using Geom::Vec2;
using Geom::OrthoRectFloat;

namespace Game{
	constexpr std::string_view ChamberRegionFieldName = "region";

	class SpaceCraft;
}

export namespace Game{
	constexpr float TileSize = 8.0f;

	struct ChamberBase{
		Geom::OrthoRectInt gridBound{};
		unsigned id{};

		[[nodiscard]] constexpr OrthoRectFloat getEntityBound() const noexcept{
			return gridBound.as<float>().scl(TileSize, TileSize);
		}
	};

	template <typename Entity>
	struct ChamberTile;

	template <typename Entity>
	struct Chamber : ChamberBase, ext::json::DynamicJsonSerializable{
		using EntityType = Entity;

		std::vector<Chamber*> proximity{};

		void writeTo(ext::json::JsonValue& jval) const override{
			writeType(jval);
			jval.append(ChamberRegionFieldName, ext::json::getJsonOf(gridBound));
		}

		void readFrom(const ext::json::JsonValue& jval) override{
			ext::json::getValueTo(gridBound, jval.asObject().at(ChamberRegionFieldName));
		}

		virtual ext::Owner<Chamber*> copy(){ //TODO temp test impl, garbage
			ext::json::JsonValue jval{};
			writeTo(jval);
			Game::Chamber<Entity>* ptr = generate<Game::Chamber<Entity>>(jval);
			if(ptr)ptr->readFrom(jval);
			return ptr;
		}

		~Chamber() override = default;

		virtual void update(float delta, Entity& entity) = 0;

		virtual void draw(const Entity& entity) const = 0;

		virtual void init(const ChamberTile<Entity>* chamberTile) = 0;

		virtual void initEntityInfo(Entity& entity){}
	};

	template <typename Entity>
	struct ChamberMocker final : Chamber<Entity>{
		void update(float delta, Entity& entity) override{}

		void draw(const Entity& entity) const override{}

		void init(const ChamberTile<Entity>* chamberTile) override{}
	};

	template <typename Entity>
	struct ChamberTile : Geom::QuadTreeAdaptable<ChamberTile<Entity>, float>{
		// using EntityType = int;
		using EntityType = Entity;
		Point2 pos{};

		std::shared_ptr<Chamber<EntityType>> chamber{};

		[[nodiscard]] ChamberTile() = default;

		[[nodiscard]] explicit ChamberTile(const Point2 pos)
			: pos{pos}{}

		[[nodiscard]] ChamberTile(const Point2 pos,
			 std::shared_ptr<Chamber<EntityType>> chamber)
			: pos{pos},
			  chamber{std::move(chamber)}{}

		[[nodiscard]] bool valid() const noexcept{
			return chamber != nullptr;
		}

		[[nodiscard]] explicit operator bool() const noexcept{
			return valid();
		}

		/**
		 * @return Whether this tile works by its reference tile
		 */
		[[nodiscard]] bool refOnly() const {
			return valid() && chamber->gridBound.getSrc() != pos;
		}

		[[nodiscard]] bool isOwner() const {
			return valid() && chamber->gridBound.getSrc() == pos;
		}

		// void init() const{
		// 	if(!isOwner()) return;
		// 	// chamber->init(this);
		// }

		[[nodiscard]] Geom::Point2 getOffsetToRef() const noexcept{
			if(refOnly()){
				return pos - chamber->gridBound.getSrc();
			}

			return {};
		}

		[[nodiscard]] Geom::Point2 getPosOfRef() const noexcept{
			if(refOnly()){
				return chamber->gridBound.getSrc();
			}

			return {};
		}


		[[nodiscard]] Geom::OrthoRectInt getChamberGridBound() const noexcept{
			return valid() ? chamber->gridBound : Geom::OrthoRectInt{pos.x, pos.y, 1, 1};
		}

		[[nodiscard]] Geom::OrthoRectFloat getChamberRegion() const noexcept{
			return valid() ? chamber->getEntityBound() : getBound();
		}

		friend bool operator==(const ChamberTile& lhs, const ChamberTile& rhs) noexcept { return lhs.pos == rhs.pos; }

		friend bool operator!=(const ChamberTile& lhs, const ChamberTile& rhs) noexcept { return !(lhs == rhs); }

		[[nodiscard]] Geom::OrthoRectFloat getBound() const noexcept{
			return Geom::OrthoRectFloat{TileSize * pos.x, TileSize * pos.y, TileSize, TileSize};
		}

		bool roughIntersectWith(const ChamberTile& other) const noexcept{
			return getBound().overlap(other.getBound());
		}

		bool containsPoint(Vec2 point) const noexcept{
			return getBound().contains(point);
		}
	};


	struct ChamberMetaDataBase{};


	template <typename Entity, Concepts::Derived<ChamberMetaDataBase> TraitData = ChamberMetaDataBase>
	struct ChamberTraitFallback{
		using TraitDataType = TraitData;

		// ReSharper disable CppMemberFunctionMayBeStatic
		void update(Chamber<Entity>*, Entity&, TraitDataType&, const float) const{}

		void init(Chamber<Entity>*, const ChamberTile<Entity>* tile, TraitDataType&) const{}

		void draw(const Chamber<Entity>*, const Entity&, const TraitDataType&) const{}
		// ReSharper restore CppMemberFunctionMayBeStatic
	};


	template <
		typename Entity,
		Concepts::Derived<ChamberMetaDataBase> TraitData = ChamberMetaDataBase,
		Concepts::Derived<ChamberTraitFallback<Entity, TraitData>> Trait = ChamberTraitFallback<TraitData>>
		requires std::same_as<TraitData, typename Trait::TraitDataType>
	class ChamberVariant : public Chamber<Entity>{
	public:
		using TraitType = Trait;
		using TraitDataType = typename TraitType::TraitDataType;
		static_assert(std::is_default_constructible_v<TraitDataType>);

	protected:
		const TraitType* trait{nullptr};
		TraitDataType mataData{};

	public:
		[[nodiscard]] ChamberVariant() noexcept = default;

		explicit ChamberVariant(const TraitType& metaDataPtr)
			: trait{&metaDataPtr}{}

		~ChamberVariant() override = default;

		void update(float delta, Entity& entity) override{
			trait->update(this, entity, mataData, delta);
		}

		void draw(const Entity& entity) const override{
			trait->draw(this, entity, mataData);
		}

		void init(const ChamberTile<Entity>* chamberTile) override{
			trait->init(this, chamberTile, mataData);
		}
	};
	
	// template <typename MetadataType, Concepts::Derived<ChamberTraitFallback<MetadataType>> Trait = ChamberTraitFallback<
	// 	MetadataType>>
	// ChamberVariant(const Trait&) -> ChamberVariant<typename Trait::TraitDataType, Trait>;

	/**
	 * @brief MetaData Wrapper. Used for chamber complex constructor with Type Erasure
	 */
	template <typename Entity> 
	struct ChamberFactory{
		using EntityType = Entity;
		virtual ~ChamberFactory() = default;

		std::string factoryName{};

		virtual void buildDesc(UI::Table& table) const{}

		virtual std::unique_ptr<Chamber<Entity>> genChamber() const{
			return std::unique_ptr<Chamber<Entity>>(nullptr);
		}

		virtual ChamberTile<Entity> genChamberTile(const Geom::OrthoRectInt region) const{
			ChamberTile<Entity> tile{};
			tile.chamber = genChamber();
			tile.pos = region.getSrc();

			if(tile.chamber){
				tile.chamber->gridBound = region;
			}else{
				if(region.area() != 1){
					throw ext::IllegalArguments{"Emply Tile Should Always Have Area of 1!"};
				}
			}

			return tile;
		}

		static ChamberTile<Entity> genEmptyTile(const Point2 pos){
			return {pos};
		}
	};
	
	template <typename Entity> 
	struct MockChamberFactory final : ChamberFactory<Entity>{
		[[nodiscard]] constexpr MockChamberFactory() = default;

		std::unique_ptr<Chamber<Entity>> genChamber() const override{
			return std::make_unique<ChamberMocker<Entity>>();
		}
	};
	
	template <typename Entity> 
	struct TurretChamberFactory : ChamberFactory<Entity>{
		struct TurretChamberData : ChamberMetaDataBase{
			Vec2 targetPos{};
			float reload{};
		};

		struct TurretChamberTrait : ChamberTraitFallback<TurretChamberData>{
			float reloadTime{};

			void update(Chamber<Entity>* chamber, typename ChamberFactory<Entity>::template TraitDataType<Entity>& data, const float delta) const{
				chamber->update(delta);
				data.reload += delta;
				if(data.reload > reloadTime){
					data.reload = 0;
				}
			}
		} baseTraitTest;


		template <Concepts::Derived<TurretChamberTrait> Trait = TurretChamberTrait>
		using ChamberType = ChamberVariant<typename Trait::TraitDataType, TurretChamberTrait>;

		std::unique_ptr<Chamber<Entity>> genChamber() const override{
			return std::make_unique<ChamberType<>>(baseTraitTest);
		}
	};

	template <typename Entity>
	struct ChamberHasher{
		size_t operator()(const Game::ChamberTile<Entity>& v) const noexcept{
			return v.pos.hash_value();
		}
	};

	template <typename Entity>
	struct ChamberJsonSrl{
		// static constexpr std::string_view DstToRef = "relativeRefPos";
		static constexpr std::string_view OwnsChamber = "ownsChamber";
		static constexpr std::string_view Chamber = "chamber";
		static constexpr Game::MockChamberFactory<Entity> mockFactory{};

		static void write(ext::json::JsonValue& jval, const Game::ChamberTile<Entity>& data){
			ext::json::append(jval, ext::json::keys::Pos, data.pos);

			if(data.isOwner()){
				ext::json::JsonValue chamberJval{};
				chamberJval.asObject();
				data.chamber->writeType(chamberJval);
				data.chamber->writeTo(chamberJval);
				jval.append(Chamber, chamberJval);
			}
		}

		static void read(const ext::json::JsonValue& jval, Game::ChamberTile<Entity>& data){
			ext::json::read(jval, ext::json::keys::Pos, data.pos);

			const auto& map = jval.asObject();

			if(const auto itr = map.find(Chamber); itr != map.end()){
				const auto& chamberJval = itr->second;
				data.chamber.reset(data.chamber->template generate<Game::Chamber<Entity>>(chamberJval));
				if(data.chamber){
					data.chamber->readFrom(chamberJval);
				}else{
					data.chamber = mockFactory.genChamber();

					if(chamberJval.asObject().contains(Game::ChamberRegionFieldName)){
						ext::json::getValueTo(data.chamber->gridBound, chamberJval.asObject().at(Game::ChamberRegionFieldName));
					}else{
						data.chamber->gridBound.setSrc(data.pos);
					}

				}
			}
		}
	};
}





