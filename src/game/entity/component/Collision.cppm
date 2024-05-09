//
// Created by Matrix on 2024/3/21.
//

export module Game.Entity.Collision;

export import Geom.Shape.RectBox;
import Geom.Transform;
import Geom.Vector2D;
import Geom.Rect_Orthogonal;
import Geom.Matrix3D;
import Math;
import Geom;

//TODO remove the IO to other place
import OS.File;

import std;

export namespace Game{

	/**
	 * \brief Used of CCD precision, the larger - the slower and more accurate
	 */
	constexpr float ContinousTestScl = 1.5f;

	struct CollisionData {
		Geom::Vec2 intersection{Geom::SNAN2};
		int subjectSubBoxIndex{};
		int objectSubBoxIndex{};

		[[nodiscard]] bool valid() const{
			return !intersection.isNaN();
		}

		void set(const Geom::Vec2 intersection, const int subjectSubBoxIndex, const int objectSubBoxIndex){
			this->intersection = intersection;
			this->objectSubBoxIndex = objectSubBoxIndex;
			this->subjectSubBoxIndex = subjectSubBoxIndex;
		}

		void swapIndex(){
			std::swap(subjectSubBoxIndex, objectSubBoxIndex);
		}
	};

	struct HitBoxFragmentData {
		Geom::Transform relaTrans{};
		Geom::RectBox original{};

		HitBoxFragmentData() = default;

		HitBoxFragmentData(const Geom::Transform relaTrans, const Geom::RectBox& original)
			: relaTrans(relaTrans),
			  original(original){
		}

		void read(std::istream& reader){
			reader.read(reinterpret_cast<char*>(&this->relaTrans), sizeof Geom::Transform);
			reader.read(reinterpret_cast<char*>(static_cast<Geom::RectBoxBrief*>(&this->original)), sizeof Geom::RectBoxBrief);
		}

		void write(std::ostream& writer) const{ //TODO rectbox opt
			writer.write(reinterpret_cast<const char*>(&this->relaTrans), sizeof Geom::Transform);
			writer.write(reinterpret_cast<const char*>(static_cast<const Geom::RectBoxBrief*>(&this->original)), sizeof Geom::RectBoxBrief);
		}
	};

	struct HitBox {
		struct BoxStateData : HitBoxFragmentData{
			mutable Geom::RectBox temp{};

			constexpr void reset() const{
				temp = original;
			}

			constexpr void trans(const Geom::Vec2 vec2) const{
				temp.move(vec2, original);
			}

			BoxStateData() = default;

			BoxStateData(const Geom::Transform& relaTrans, const Geom::RectBox& original, const Geom::RectBox& temp)
				: HitBoxFragmentData(relaTrans, original),
				  temp(temp){
			}
		};

		std::vector<BoxStateData> hitBoxGroup{};
		mutable Geom::Transform trans{};

		//Bolow are transient fields

		/** @brief CCD-traces size.*/
		int sizeTrace{};
		/** @brief CCD-traces spacing*/
		Geom::Vec2 transitionCCD{};
		/** @brief CCD-traces clamp size.*/
		mutable std::atomic<int> sizeTraceClamped{};

		Geom::OrthoRectFloat maxTransientBound{};
		Geom::OrthoRectFloat maxBound{};

		HitBox() = default;

		HitBox(const HitBox& other)
			: hitBoxGroup(other.hitBoxGroup),
			  trans(other.trans){
			sizeTraceClamped = sizeTrace;
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

			const float cos = Math::cosDeg(this->trans.rot);
			const float sin = Math::sinDeg(this->trans.rot);

			for(auto& group : hitBoxGroup){
				Geom::Vec2 trans = group.relaTrans.vec;

				group.original.update(this->trans.vec + trans.rotate(cos, sin), this->trans.rot + group.relaTrans.rot);
				group.reset();
			}

			updateMaxBound();
		}

		[[nodiscard]] float getAvgSizeSqr() const{
			return maxTransientBound.maxDiagonalSqLen() * 0.35f;
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
			sizeTrace = seg + 1;
			sizeTraceClamped = sizeTrace;
			transitionCCD = -velo;
		}

		void scl(const Geom::Vec2 scl){
			for (auto& data : hitBoxGroup){
				data.original.offset *= scl;
				data.original.sizeVec2 *= scl;
			}
		}

		void updateMaxBound(){
			float minX = std::numeric_limits<float>::max();
			float minY = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float maxY = std::numeric_limits<float>::lowest();

			for(auto& boxData : hitBoxGroup){
				minX = Math::min(minX, boxData.original.maxOrthoBound.getSrcX());
				minY = Math::min(minY, boxData.original.maxOrthoBound.getSrcY());
				maxX = Math::max(maxX, boxData.original.maxOrthoBound.getEndX());
				maxY = Math::max(maxY, boxData.original.maxOrthoBound.getEndY());
			}

			maxBound.setVert(minX, minY, maxX, maxY);

			maxTransientBound = maxBound;

			if(sizeTrace == 0)return;

			Geom::Vec2 trans = transitionCCD;
			trans.scl(static_cast<float>(sizeCCD()));

			if(trans.x > 0){
				(void)maxBound.addWidth(trans.x);
			}else{
				maxBound.setSrcX(minX + trans.x);
			}

			if(trans.y > 0){
				(void)maxBound.addHeight(trans.y);
			}else{
				maxBound.setSrcY(minY + trans.y);
			}
		}

		void init(const std::vector<HitBoxFragmentData>& sample){
			hitBoxGroup.resize(sample.size());

			for(int i = 0; i < sample.size(); ++i){
				auto& boxData = hitBoxGroup.at(i);
				auto& [tgtTrans, tgtBox] = sample.at(i);
				boxData.temp = boxData.original = tgtBox;
				boxData.relaTrans = tgtTrans;
			}
		}

		void init(const Geom::RectBox& sample, const Geom::Transform trans = {}){
			hitBoxGroup.emplace_back(trans, sample, sample);
		}

		constexpr void reset() const noexcept{
			for(auto& group : hitBoxGroup){
				group.reset();
			}
		}

		[[nodiscard]] bool enableCCD() const noexcept{
			return sizeTraceClamped > 0;
		}

		void clampCCDTo(const int index) const noexcept{
			if(index>= sizeTraceClamped)return;
			sizeTraceClamped = index;
		}

		void resizeCCD(const int tgtSize) const noexcept{
			sizeTraceClamped = tgtSize;
		}

		[[nodiscard]] int sizeCCD() const noexcept{
			return Math::max(static_cast<int>(sizeTraceClamped), 1);
		}

		[[nodiscard]] bool emptyCCD() const noexcept{
			return sizeTraceClamped == 0;
		}

		[[nodiscard]] std::size_t size() const noexcept{
			return hitBoxGroup.size();
		}

		/**
		 * @code
		 * | Index ++ <=> Distance ++ <=> Backwards ++
		 * + ----0----  ...  ----maxSize--->
		 * |    End              Initial
		 * |   State              State
		 * @endcode
		 * @param transIndex [0, @link sizeCCD() @endlink)
		 * @return Translation
		 */
		[[nodiscard]] constexpr Geom::Vec2 transTo(const int transIndex) const{
			Geom::Vec2 trans = transitionCCD;
			trans.scl(sizeTrace - transIndex - 1.0f);
			for(auto& rectBox : hitBoxGroup){
				rectBox.trans(trans);
			}

			return trans;
		}

		[[nodiscard]] Geom::Vec2 getAvgEdgeNormal(const CollisionData data) const{
			return Geom::avgEdgeNormal(data.intersection, hitBoxGroup.at(data.objectSubBoxIndex).original);
		}

		[[nodiscard]] bool collideWithRough(const HitBox& other) const{
			return maxBound.overlap(other.maxBound);
		}

		[[nodiscard]] CollisionData collideWithExact(const HitBox& other, const bool requiresIntersection = true) const{
			CollisionData collisionData{};

			int objectLastIndex = 0;
			for(int subjectIndex = 0; subjectIndex < sizeCCD(); ++subjectIndex){
				//Calculate Move Step
				const float curRatio = (static_cast<float>(subjectIndex) + 1) / static_cast<float>(sizeCCD());
				const int objectCurrentIndex = //The subject's size may shrink, cause the ratio larger than 1, resulting in array index out of bound
					Math::min(
						Math::ceil(curRatio * static_cast<float>(other.sizeCCD())),
						other.sizeCCD());

				for(int objectIndex = objectLastIndex; objectIndex < objectCurrentIndex; ++objectIndex){
					//Perform CCD approach
					const auto transSubject = this->transTo(subjectIndex);
					const auto transObject = other.transTo(objectIndex);

					//Rough collision test 1;
					if(!maxTransientBound.overlap(other.maxTransientBound, transSubject, transObject))continue;

					//Collision Test
					//Is a small local quad tree necessary?
					//Should less than 6quads collide with 6quads
					for(auto [subjectBoxIndex, subject] : this->hitBoxGroup | std::ranges::views::transform(&BoxStateData::temp) | std::views::enumerate){
						for(auto [objectBoxIndex, object] : other.hitBoxGroup | std::ranges::views::transform(&BoxStateData::temp) | std::views::enumerate){
							if(!subject.overlapRough(object) || !subject.overlapExact(object))continue;
							if(requiresIntersection){
								collisionData.set(Geom::rectAvgIntersection(subject, object), subjectBoxIndex, objectBoxIndex);
							}else{
								collisionData.intersection = Geom::ZERO;
							}

							this->clampCCDTo(subjectIndex);
							other.clampCCDTo(objectIndex);

							//TODO is this position clamp necessary？
							this->trans.vec.mulAdd(transitionCCD, static_cast<float>(subjectIndex));
							other.trans.vec.mulAdd(other.transitionCCD, static_cast<float>(objectIndex));

							return collisionData;
						}
					}
				}

				objectLastIndex = objectCurrentIndex;
			}

			return collisionData;
		}

		[[nodiscard]] constexpr float getRotationalInertia(const float mass, const float scale = 1 / 12.0f, const float lengthRadiusRatio = 0.25f) const {
			return std::accumulate(hitBoxGroup.begin(), hitBoxGroup.end(),
				0.0f, [mass, scale, lengthRadiusRatio](const float val, const BoxStateData& pair){
				return val + pair.original.getRotationalInertia(mass, scale, lengthRadiusRatio) * pair.relaTrans.vec.length2();
			});
		}

		[[nodiscard]] bool contains(const Geom::Vec2 vec2) const{
			return std::ranges::any_of(hitBoxGroup, [vec2](const BoxStateData& data){
				return data.original.contains(vec2);
			});
		}
	};

	template <Concepts::Derived<HitBoxFragmentData> T>
	void flipX(std::vector<T>& datas){
		const std::size_t size = datas.size();

		std::ranges::copy(datas, std::back_inserter(datas));

		for(int i = 0; i < size; ++i){
			auto& dst = datas.at(i + size);
			dst.relaTrans.vec.y *= -1;
			dst.relaTrans.rot *= -1;
			dst.relaTrans.rot = Math::Angle::getAngleInPi2(dst.relaTrans.rot);
			dst.original.offset.y *= -1;
			dst.original.sizeVec2.y *= -1;
		}
	}

	void scl(std::vector<HitBoxFragmentData>& datas, const float scl){
		for (auto& data : datas){
			data.original.offset *= scl;
			data.original.sizeVec2 *= scl;
			data.relaTrans.vec *= scl;
		}
	}

	void flipX(HitBox& datas){
		flipX(datas.hitBoxGroup);
		for (auto& stateData : datas.hitBoxGroup){
			stateData.reset();
		}
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

