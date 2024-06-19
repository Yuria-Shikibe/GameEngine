module;

#include <../src/code-gen/ReflectData_Builtin.hpp>

export module Game.Entity.Collision;

export import Geom.Shape.RectBox;
import Geom.Transform;
import Geom.Vector2D;
import Geom.Rect_Orthogonal;
import Geom.Matrix3D;
import Math;
import Geom;

import ext.json;
import ext.json.io;
export import Core.IO.Specialized;

import ext.StaticReflection;


//TODO remove the IO to other place
import OS.File;

import std;

export namespace Game{

	//TODO no mutable
	//TODO json srl
	/**
	 * \brief Used of CCD precision, the larger - the slower and more accurate
	 */
	constexpr float ContinousTestScl = 1.25f;

	struct CollisionData {
		Geom::Vec2 intersection{Geom::SNAN2};
		int subjectSubBoxIndex{};
		int objectSubBoxIndex{};

		[[nodiscard]] bool valid() const{
			return !intersection.isNaN();
		}

		constexpr void set(const Geom::Vec2 intersection, const int subjectSubBoxIndex, const int objectSubBoxIndex) noexcept{
			this->intersection = intersection;
			this->objectSubBoxIndex = objectSubBoxIndex;
			this->subjectSubBoxIndex = subjectSubBoxIndex;
		}

		void swapIndex() noexcept{
			std::swap(subjectSubBoxIndex, objectSubBoxIndex);
		}
	};

	struct HitBoxFragment {
		/** @brief Local Transform */
		Geom::Transform trans{};

		/** @brief Raw Box Data */
		Geom::RectBox box{};

		[[nodiscard]] constexpr HitBoxFragment() noexcept = default;

		HitBoxFragment(const Geom::Transform relaTrans, const Geom::RectBox& original) noexcept
			: trans(relaTrans),
			  box(original){
		}

		void read(std::istream& reader){
			reader.read(reinterpret_cast<char*>(&this->trans), sizeof Geom::Transform);
			reader.read(reinterpret_cast<char*>(static_cast<Geom::RectBoxIdentity*>(&this->box)), sizeof Geom::RectBoxIdentity);
		}

		void write(std::ostream& writer) const{ //TODO rectbox opt
			writer.write(reinterpret_cast<const char*>(&this->trans), sizeof Geom::Transform);
			writer.write(reinterpret_cast<const char*>(static_cast<const Geom::RectBoxIdentity*>(&this->box)), sizeof Geom::RectBoxIdentity);
		}
	};

	class HitBox {
	public:
		static constexpr std::string_view HitboxFileSuffix{".hitbox"};

	private:
		/** @brief CCD-traces size.*/
		int size_CCD{};
		/** @brief CCD-traces spacing*/
		Geom::Vec2 displacement_CCD{};
		/** @brief CCD-traces clamp size. shrink only!*/
		mutable std::atomic<int> sizeClamped_CCD{};

		// Geom::OrthoRectFloat maxBound_CCD{};

	public:
		Geom::RectBoxBrief wrapBound_CCD;


		Geom::OrthoRectFloat maxBound{};

		std::vector<HitBoxFragment> hitBoxGroup{};
		Geom::Transform trans{};

		HitBox() = default;

		HitBox(const HitBox& other)
			: hitBoxGroup(other.hitBoxGroup),
			  trans(other.trans){
			sizeClamped_CCD = size_CCD;
		}

		HitBox(HitBox&& other) noexcept
			: hitBoxGroup(std::move(other.hitBoxGroup)),
			  trans(std::move(other.trans)){}

		HitBox& operator=(const HitBox& other){
			if(this == &other) return *this;
			hitBoxGroup = other.hitBoxGroup;
			trans = other.trans;
			return *this;
		}

		HitBox& operator=(HitBox&& other) noexcept{
			if(this == &other) return *this;
			hitBoxGroup = std::move(other.hitBoxGroup);
			trans = std::move(other.trans);
			return *this;
		}

		void updateHitbox(const Geom::Transform translation){
			this->trans = translation;

			const auto displayment = displacement_CCD * static_cast<float>(size_CCD);
			const float ang = displayment.angle();

			const float cos = Math::cosDeg(-ang);
			const float sin = Math::sinDeg(-ang);

			float minX = std::numeric_limits<float>::max();
			float minY = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float maxY = std::numeric_limits<float>::lowest();

			//TODO Simple object [box == 1] Optimization (for bullets mainly)

			for(auto& boxData : hitBoxGroup){
				boxData.box.update(boxData.trans | trans);

				const std::array verts{
					boxData.box.v0.copy().rotate(cos, sin),
					boxData.box.v1.copy().rotate(cos, sin),
					boxData.box.v2.copy().rotate(cos, sin),
					boxData.box.v3.copy().rotate(cos, sin)};

				for (auto [x, y] : verts){
					minX = Math::min(minX, x);
					minY = Math::min(minY, y);
					maxX = Math::max(maxX, x);
					maxY = Math::max(maxY, y);
				}
			}

			[[assume(minX <= maxX)]];
			[[assume(minY <= maxY)]];

			maxX += displayment.length();


			wrapBound_CCD = Geom::RectBoxBrief{
				Geom::Vec2{minX, minY}.rotate(cos, -sin),
				Geom::Vec2{maxX, minY}.rotate(cos, -sin),
				Geom::Vec2{maxX, maxY}.rotate(cos, -sin),
				Geom::Vec2{minX, maxY}.rotate(cos, -sin)
			};

			wrapBound_CCD.updateBound();
			wrapBound_CCD.updateNormal();

			maxBound = wrapBound_CCD.maxOrthoBound.shrinkBy(-displayment);
		}

		[[nodiscard]] float getAvgSizeSqr() const{
			return maxBound.maxDiagonalSqLen() * 0.35f;
		}

		/**
		 * \brief This ignores the rotation of the subject entity!
		 * Record backwards movements
		 */
		void genContinousRectBox(Geom::Vec2 velo, const float delta,
								 const float scl = ContinousTestScl) {
			velo *= delta;
			const float dst2  = velo.length2();
			const float size2 = getAvgSizeSqr();

			if(size2 < 0.025f)return;

			const int seg = Math::ceil(std::sqrtf(dst2 / size2) * scl + 0.00001f);

			velo.div(static_cast<float>(seg));
			size_CCD = seg + 1;
			sizeClamped_CCD = size_CCD;
			displacement_CCD = -velo;
		}

		void scl(const Geom::Vec2 scl){
			for (auto& data : hitBoxGroup){
				data.box.offset *= scl;
				data.box.sizeVec2 *= scl;
			}
		}

		void init(const std::vector<HitBoxFragment>& sample){
			hitBoxGroup.resize(sample.size());

			for(const auto& [index, boxData] : hitBoxGroup | std::views::enumerate){
				auto& [tgtTrans, tgtBox] = sample.at(index);
				boxData.box = tgtBox;
				boxData.trans = tgtTrans;
			}
		}

		void init(const Geom::RectBox& sample, const Geom::Transform trans = {}){
			hitBoxGroup.emplace_back(trans, sample);
		}

		[[nodiscard]] bool enableCCD() const noexcept{
			return sizeClamped_CCD > 0;
		}

		void clampCCDTo(const int index) const noexcept{
			if(index>= sizeClamped_CCD)return;
			sizeClamped_CCD = index;
		}

		void resizeCCD(const int tgtSize) const noexcept{
			sizeClamped_CCD = tgtSize;
		}

		[[nodiscard]] int sizeCCD_clamped() const noexcept{ // always >= 1
			return Math::max(sizeClamped_CCD.load(), 1);
		}

		[[nodiscard]] bool emptyCCD() const noexcept{
			return sizeClamped_CCD == 0;
		}

		[[nodiscard]] constexpr std::size_t size() const noexcept{
			return hitBoxGroup.size();
		}

		/**
		 * @code
		 * | Index ++ <=> Distance ++ <=> Backwards ++
		 * + ----0----  ...  ----maxSize--->
		 * |    End              Initial
		 * |   State              State
		 * @endcode
		 * @param transIndex [0, @link sizeCCD_clamped() @endlink)
		 * @return Translation
		 */
		[[nodiscard]] constexpr Geom::Vec2 getDisplacement_CCD(const int transIndex) const{
			return displacement_CCD * (size_CCD - transIndex - 1.0f);
		}

		static void transHitboxGroup(const std::span<HitBoxFragment> boxes, const Geom::Vec2 trans){
			for (auto && box : boxes){
				box.box.move(trans);
			}
		}

		[[nodiscard]] Geom::Vec2 getAvgEdgeNormal(const CollisionData data) const{
			return Geom::avgEdgeNormal(data.intersection, hitBoxGroup.at(data.objectSubBoxIndex).box);
		}

		[[nodiscard]] bool collideWithRough(const HitBox& other) const{
			return wrapBound_CCD.overlapRough(other.wrapBound_CCD) && wrapBound_CCD.overlapExact(other.wrapBound_CCD);
		}

		[[nodiscard]] CollisionData collideWithExact(const HitBox& other, const bool requiresIntersection = true) const{
			CollisionData collisionData{Geom::SNAN2};

			const auto subjectCCD_size = sizeCCD_clamped();
			const auto objectCCD_size = other.sizeCCD_clamped();

			Geom::OrthoRectFloat bound_subject = this->maxBound;
			Geom::OrthoRectFloat bound_object = other.maxBound;

			std::vector<Geom::RectBoxBrief> tempHitboxes{};
			tempHitboxes.reserve(hitBoxGroup.size() + other.hitBoxGroup.size());

			tempHitboxes.append_range(hitBoxGroup | std::views::transform(&HitBoxFragment::box));
			tempHitboxes.append_range(other.hitBoxGroup | std::views::transform(&HitBoxFragment::box));

			const auto rangeSubject = std::span{tempHitboxes.begin(), size()};
			const auto rangeObject = std::span{tempHitboxes.begin() + size(), tempHitboxes.end()};

			const auto maxTrans_subject = this->getDisplacement_CCD(0);
			const auto maxTrans_object = other.getDisplacement_CCD(0);

			//Move to initial stage
			for (auto& box : rangeSubject){
				box.move(maxTrans_subject);
			}

			for (auto& box : rangeObject){
				box.move(maxTrans_object);
			}

			bound_subject.move(maxTrans_subject);
			bound_object.move(maxTrans_object);

			const auto trans_subject = -this->displacement_CCD;
			const auto trans_object = -other.displacement_CCD;

			int lastCheckedIndex_subject{};
			int lastCheckedIndex_object{};

			for(int lastIndex_object{}, index_subject{}; index_subject < subjectCCD_size; ++index_subject){
				//Calculate Move Step
				const float curRatio = (static_cast<float>(index_subject) + 1) / static_cast<float>(subjectCCD_size);
				const int objectCurrentIndex = //The subject's size may shrink, cause the ratio larger than 1, resulting in array index out of bound
					Math::ceil(curRatio * static_cast<float>(objectCCD_size));

				for(int index_object = lastIndex_object; index_object < objectCurrentIndex; ++index_object){
					//Perform CCD approach

					//Rough collision test 1;
					if(!bound_subject.overlap(bound_object)){
						bound_subject.move(trans_subject);
						bound_object.move(trans_object);

						continue;
					}

					bound_subject.move(trans_subject);
					bound_object.move(trans_object);

					//Rough test passed, move boxes to last valid position
					for (auto& box : rangeSubject){
						box.move(trans_subject * (index_subject - lastCheckedIndex_subject));
					}

					for (auto& box : rangeObject){
						box.move(trans_object * (index_object - lastCheckedIndex_object));
					}

					//Collision Test
					//TODO O(n^2) Is a small local quad tree necessary?
					for(auto&& [subjectBoxIndex, subject] : rangeSubject | std::views::enumerate){
						for(auto&& [objectBoxIndex, object] : rangeObject | std::views::enumerate){
							if(subjectCCD_size > this->sizeCCD_clamped() || objectCCD_size > other.sizeCCD_clamped())return collisionData;

							if(!subject.overlapRough(object) || !subject.overlapExact(object))continue;

							if(requiresIntersection){
								collisionData.set(Geom::rectAvgIntersection(subject, object), subjectBoxIndex, objectBoxIndex);
							}else{
								collisionData.intersection = Geom::SNAN2;
							}


							this->clampCCDTo(index_subject);
							other.clampCCDTo(index_object);

							return collisionData;
						}
					}

					lastCheckedIndex_subject = index_subject;
					lastCheckedIndex_object = index_object;
				}

				lastIndex_object = objectCurrentIndex;
			}

			return collisionData;
		}

		//TODO shrink CCD bound
		//TODO shrink requires atomic...
		void fetchToLastClampPosition(){
			this->trans.vec.addScaled(displacement_CCD, static_cast<float>(sizeClamped_CCD.load()));
		}

		[[nodiscard]] constexpr float getRotationalInertia(const float mass, const float scale = 1 / 12.0f, const float lengthRadiusRatio = 0.25f) const {
			return std::accumulate(hitBoxGroup.begin(), hitBoxGroup.end(),
				1.0f, [mass, scale, lengthRadiusRatio](const float val, const HitBoxFragment& pair){
				return val + pair.box.getRotationalInertia(mass, scale, lengthRadiusRatio) + mass * pair.trans.vec.length2();
			});
		}

		[[nodiscard]] bool contains(const Geom::Vec2 vec2) const{
			return std::ranges::any_of(hitBoxGroup, [vec2](const auto& data){
				return data.box.contains(vec2);
			});
		}
	};

	template <Concepts::Derived<HitBoxFragment> T>
	void flipX(std::vector<T>& datas){
		const std::size_t size = datas.size();

		std::ranges::copy(datas, std::back_inserter(datas));

		for(int i = 0; i < size; ++i){
			auto& dst = datas.at(i + size);
			dst.trans.vec.y *= -1;
			dst.trans.rot *= -1;
			dst.trans.rot = Math::Angle::getAngleInPi2(dst.trans.rot);
			dst.box.offset.y *= -1;
			dst.box.sizeVec2.y *= -1;
		}
	}

	void scl(std::vector<HitBoxFragment>& datas, const float scl){
		for (auto& data : datas){
			data.box.offset *= scl;
			data.box.sizeVec2 *= scl;
			data.trans.vec *= scl;
		}
	}

	void flipX(HitBox& datas){
		flipX(datas.hitBoxGroup);
	}

	void read(const OS::File& file, HitBox& box){
		box.hitBoxGroup.clear();
		int size = 0;

		std::ifstream reader{file.getPath(), std::ios::binary | std::ios::in};

		reader.read(reinterpret_cast<char*>(&size), sizeof(size));

		box.hitBoxGroup.resize(size);

		for (auto& data : box.hitBoxGroup){
			data.read(reader);
		}

	}
}

export template<>
struct ext::json::JsonSerializator<Geom::RectBox>{
	static void write(ext::json::JsonValue& jsonValue, const Geom::RectBox& data){
		ext::json::append(jsonValue, ext::json::keys::Size2D, data.sizeVec2);
		ext::json::append(jsonValue, ext::json::keys::Offset, data.offset);
		ext::json::append(jsonValue, ext::json::keys::Trans, data.transform);
	}

	static void read(const ext::json::JsonValue& jsonValue, Geom::RectBox& data){
		ext::json::read(jsonValue, ext::json::keys::Size2D, data.sizeVec2);
		ext::json::read(jsonValue, ext::json::keys::Offset, data.offset);
		ext::json::read(jsonValue, ext::json::keys::Trans, data.transform);
	}
};

export template<>
struct ext::json::JsonSerializator<Game::HitBoxFragment>{
	static void write(ext::json::JsonValue& jsonValue, const Game::HitBoxFragment& data){
		ext::json::append(jsonValue, ext::json::keys::Trans, data.trans);
		ext::json::append(jsonValue, ext::json::keys::Data, data.box);
	}

	static void read(const ext::json::JsonValue& jsonValue, Game::HitBoxFragment& data){
		ext::json::read(jsonValue, ext::json::keys::Trans, data.trans);
		ext::json::read(jsonValue, ext::json::keys::Data, data.box);
	}
};

export template<>
struct ext::json::JsonSerializator<Game::HitBox>{
	static void write(ext::json::JsonValue& jsonValue, const Game::HitBox& data){
		ext::json::JsonValue boxData{};
		ext::json::JsonSrlContBase_vector<decltype(data.hitBoxGroup)>::write(boxData, data.hitBoxGroup);

		ext::json::append(jsonValue, ext::json::keys::Data, std::move(boxData));
		ext::json::append(jsonValue, ext::json::keys::Trans, data.trans);
	}

	static void read(const ext::json::JsonValue& jsonValue, Game::HitBox& data){
		const auto& map = jsonValue.asObject();
		const ext::json::JsonValue boxData = map.at(ext::json::keys::Data);
		ext::json::JsonSrlContBase_vector<decltype(data.hitBoxGroup)>::read(boxData, data.hitBoxGroup);
		ext::json::read(jsonValue, ext::json::keys::Trans, data.trans);
		data.updateHitbox(data.trans);
	}
};
