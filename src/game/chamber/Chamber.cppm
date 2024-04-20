//
// Created by Matrix on 2024/4/18.
//

export module Game.Chamber;

export import Geom.Rect_Orthogonal;
export import Geom.Vector2D;
import Concepts;
import ext.RuntimeException;
import std;

using Geom::Point2;
using Geom::Vec2;
using Geom::OrthoRectFloat;

export namespace Game{
	struct ChamberTile;


	struct Chamber{
		Geom::OrthoRectInt region{};
		unsigned id{};
		float unitLength{};

		std::vector<Chamber*> proximity{};

		OrthoRectFloat getChamberBound() const{
			return region.as<float>().scl(unitLength, unitLength);;
		}

		virtual ~Chamber() = default;

		virtual void update(float delta) = 0;

		virtual void draw() const = 0;

		virtual void init(const ChamberTile* chamberTile) = 0;
	};


	struct ChamberTile{
		Point2 pos{};
		/** Usage for multi tile*/
		ChamberTile* referenceTile{nullptr};
		float unitLength{}; //TODO Should this be constexpr??

		std::unique_ptr<Chamber> chamber{};

		[[nodiscard]] bool valid() const {
			//Not chain query, only one-depth reference is allowed
			return ownsChamber() || (referenceTile && referenceTile->ownsChamber());
		}

		[[nodiscard]] bool refOnly() const {
			//Not chain query, only one-depth reference is allowed
			return !ownsChamber() && referenceTile && referenceTile->ownsChamber();
		}

		[[nodiscard]] bool ownsChamber() const{
			return !referenceTile && static_cast<bool>(chamber);
		}

		[[nodiscard]] bool isSubTile() const{
			return static_cast<bool>(referenceTile);
		}

		template <Concepts::InvokeNullable<void(const ChamberTile&)> DrawPred = std::nullptr_t>
		void draw(DrawPred&& fallbackDrawer = nullptr) const{
			if(!ownsChamber()){
				if constexpr(!std::same_as<DrawPred, std::nullptr_t>){
					fallbackDrawer(*this);
				}

				return;
			}
			chamber->draw();
		}

		void init(const float unitLength){
			this->unitLength = unitLength;

			if(!ownsChamber()) return;
			chamber->unitLength = unitLength;
			chamber->init(this);
		}

		void setReference(ChamberTile& referenceTile){
			if(ownsChamber() || !referenceTile.ownsChamber() || !referenceTile.chamber->region.containsPos_edgeInclusive(pos)){
				throw ext::IllegalArguments{std::format("Wrongly Set a refernece tile")};
			}

			this->referenceTile = &referenceTile;
			chamber.reset();
		}

		void setReference(std::nullptr_t) noexcept{
			this->referenceTile = nullptr;
			chamber.reset();
		}

		[[nodiscard]] Geom::Point2 getOffsetToRef() const {
			if(isSubTile()){
				return pos - referenceTile->pos;
			}

			return {};
		}

		[[nodiscard]] Geom::OrthoRectFloat getTileBound() const noexcept{
			return {unitLength * pos.x, unitLength * pos.y, unitLength, unitLength};
		}

		[[nodiscard]] Geom::OrthoRectInt getChamberRegion() const noexcept{
			return ownsChamber() ? chamber->region : Geom::OrthoRectInt{pos.x, pos.y, 1, 1};
		}

		[[nodiscard]] Geom::OrthoRectFloat getChamberRealRegion() const noexcept{
			return ownsChamber() ? chamber->getChamberBound() : getTileBound();
		}

		friend bool operator==(const ChamberTile& lhs, const ChamberTile& rhs) noexcept { return lhs.pos == rhs.pos; }

		friend bool operator!=(const ChamberTile& lhs, const ChamberTile& rhs) noexcept { return !(lhs == rhs); }
	};


	struct ChamberMetaDataBase{};


	template <Concepts::Derived<ChamberMetaDataBase> TraitData = ChamberMetaDataBase>
	struct ChamberTraitFallback{
		using TraitDataType = TraitData;

		// ReSharper disable CppMemberFunctionMayBeStatic
		void update(Chamber*, TraitDataType&, const float) const{}

		void init(Chamber* chamber, const ChamberTile* tile, TraitDataType&) const{
			// chamber->region = tile->region;
		}

		void draw(const Chamber*, const TraitDataType&) const{}

		// ReSharper restore CppMemberFunctionMayBeStatic
	};


	template <
		Concepts::Derived<ChamberMetaDataBase> TraitData = ChamberMetaDataBase,
		Concepts::Derived<ChamberTraitFallback<TraitData>> Trait = ChamberTraitFallback<TraitData>>
		requires std::same_as<TraitData, typename Trait::TraitDataType>
	class ChamberVariant : public Chamber{
	public:
		using TraitType = Trait;
		using TraitDataType = typename TraitType::TraitDataType;
		static_assert(std::is_default_constructible_v<TraitDataType>);

	protected:
		const TraitType* trait{nullptr};
		TraitDataType mataData{};

	public:
		explicit ChamberVariant(const TraitType& metaDataPtr)
			: trait{&metaDataPtr}{}

		~ChamberVariant() override = default;

		void update(float delta) override{
			trait->update(this, mataData, delta);
		}

		void draw() const override{
			trait->draw(this, mataData);
		}

		void init(const ChamberTile* chamberTile) override{
			trait->init(this, chamberTile, mataData);
		}
	};



	template <typename MetadataType, Concepts::Derived<ChamberTraitFallback<MetadataType>> Trait = ChamberTraitFallback<
		MetadataType>>
	ChamberVariant(const Trait&) -> ChamberVariant<typename Trait::TraitDataType, Trait>;

	/**
	 * @brief MetaData Wrapper. Used for chamber complex constructor with Type Erasure
	 */
	struct ChamberFactory{
		virtual ~ChamberFactory() = default;

		virtual std::unique_ptr<Chamber> genChamber() const{
			return std::unique_ptr<Chamber>(nullptr);
		}

		virtual ChamberTile genChamberTile(const Geom::OrthoRectInt region) const{
			ChamberTile tile{};
			tile.chamber = genChamber();
			tile.pos = region.getSrc();

			if(tile.chamber){
				tile.chamber->region = region;
			}else{
				if(region.area() != 1){
					throw ext::IllegalArguments{"Emply Tile Should Always Have Area of 1!"};
				}
			}

			return tile;
		}

		static ChamberTile genEmptyTile(const Point2 pos){
			return {pos};
		}
	};


	struct TurretChamberFactory : ChamberFactory{
		struct TurretChamberData : ChamberMetaDataBase{
			Vec2 targetPos{};
			float reload{};
		};


		struct TurretChamberTrait : ChamberTraitFallback<TurretChamberData>{
			float reloadTime{};

			void update(Chamber* chamber, TraitDataType& data, const float delta) const{
				chamber->update(delta);
				data.reload += delta;
				if(data.reload > reloadTime){
					data.reload = 0;
				}
			}
		} baseTraitTest;


		template <Concepts::Derived<TurretChamberTrait> Trait = TurretChamberTrait>
		using ChamberType = ChamberVariant<typename Trait::TraitDataType, TurretChamberTrait>;

		std::unique_ptr<Chamber> genChamber() const override{
			return std::make_unique<ChamberType<>>(baseTraitTest);
		}
	};


	TurretChamberFactory::TurretChamberTrait baseTraitTest{};

	TurretChamberFactory::ChamberType<> testChamberType{baseTraitTest};
}


export
template <>
struct std::hash<Game::ChamberTile>{
	size_t operator()(const Game::ChamberTile& v) const noexcept{
		return v.pos.hash_value();
	}
};
