//
// Created by Matrix on 2023/11/19.
//

module ;

export module Core.Batch;

import std;
import GL.Blending;
import GL.Constants;
export import GL.Mesh;
export import GL.Shader;
export import GL.Texture;
import ext.RuntimeException;

import Geom.Matrix3D;

using namespace GL;

namespace Core{
	export class Batch
	{
	protected:
		Mesh mesh1{};
		Mesh mesh2{};

		float* buffer1{nullptr};
		float* buffer2{nullptr};

		bool swapped = false;
		const Texture* lastTexture = nullptr;

		unsigned index{0};

		ShaderProgram* generalShader = nullptr;
		ShaderProgram* customShader = nullptr;

		Blending blending = Blendings::Normal;

		/**
		 * \brief From Viewport Size to normalized [-1, 1]
		 */
		const Geom::Matrix3D* projection = nullptr;
		Geom::Matrix3D localToWorld{};

		// const Geom::Matrix3D*

		const Geom::Matrix3D* projectionFallback = nullptr;

	public:
		[[nodiscard]] Batch() = default;

		Batch(const Batch& other) = delete;

		Batch& operator=(const Batch& other) = delete;

		constexpr float* getCurrentMappedPtr() const noexcept{
			return swapped ? buffer1 : buffer2;
		}

		constexpr float* getCurrentWritePtr() const noexcept{
			return getCurrentMappedPtr() + index;
		}

		constexpr GL::Mesh& getCurrentMesh() noexcept{
			return swapped ? mesh1 : mesh2;
		}

		constexpr const GL::Mesh& getCurrentMesh() const noexcept{
			return swapped ? mesh1 : mesh2;
		}

		constexpr void swapBuffer() noexcept{
			swapped = !swapped;
		}

		[[nodiscard]] unsigned getIndex() const noexcept{ return index; }

		constexpr void push(const unsigned count) noexcept{
			index += count;
		}

		virtual void reset(){
			localToWorld.idt();
			projectionFallback = nullptr;
			lastTexture = nullptr;
		}

		void setLocalToWorld(const Geom::Matrix3D& mat){
			localToWorld = mat;
		}

		void resetLocalToWorld(){
			flush();
			localToWorld = Geom::MAT3_IDT;
		}

		/**
		 * @brief notice that this will change flush the context;
		 */
		[[nodiscard]] Geom::Matrix3D& modifyGetLocalToWorld() noexcept{
			flush();
			return localToWorld;
		}
		[[nodiscard]] const Geom::Matrix3D& getLocalToWorld() const noexcept{ return localToWorld; }

		[[nodiscard]] bool hasShader() const {
			return customShader || generalShader;
		}

		[[nodiscard]] bool hasCustomShader() const {
			return customShader;
		}

		[[nodiscard]] ShaderProgram* getGeneralShader() const {
			return generalShader;
		}

		[[nodiscard]] const Texture* getTexture() const{
			return lastTexture;
		}

		void setCustomShader(ShaderProgram* shader) noexcept{
			customShader = shader;
		}

		void setCustomShader() {
			customShader = nullptr;
		}

		[[nodiscard]] ShaderProgram* getCustomShader() const{ return customShader; }

		void setupBlending() const {
			blending.apply();
		}

		void switchCustomShader(ShaderProgram* shader = nullptr) {
			flush();

			setCustomShader(shader);
		}

		void clearCustomShader(const bool flushContext = true) {
			if(flushContext)flush();

			customShader = nullptr;
			generalShader->bind();
		}

		void switchBlending(const Blending blend = GL::Blendings::Normal){
			if(blending != blend){
				flush();
				blending = blend;
				blending.apply();
			}
		}

		[[nodiscard]] const Mesh& getMesh() const {
			return mesh1;
		}

		void setProjection(const Geom::Matrix3D* porj){
			projection = porj;
		}

		/**
		 * @warning Projection Should have a long enough lifetime
		 */
		void setProjection(const Geom::Matrix3D& porj){
			projection = &porj;
		}

		void beginTempProjection(const Geom::Matrix3D& porj) {
			if(&porj == projection || porj == *projection)return;

			if(projectionFallback != nullptr) {
				throw ext::IllegalArguments{"Cannot begin porjection before end the former one!"};
			}

			flush();

			projectionFallback = projection;
			projection = &porj;
		}

		void endTempProjection() {
			if(projectionFallback == nullptr)return;

			flush();

			projection = projectionFallback;
			projectionFallback = nullptr;
		}

		[[nodiscard]] const Geom::Matrix3D* getProjection() const{
			return projection;
		}

		void bindShader() const{
			if(hasCustomShader()){
				customShader->bind();
			}else{
				generalShader->bind();
			}
		}

		void applyShader() const{
			if(hasCustomShader()){
				//TODO this is really dangerous!
				customShader->applyDynamic(generalShader->getUniformer(), true);
			}else{
				generalShader->apply();
			}
		}

		virtual ~Batch() = default;

		virtual void flush() = 0;

		[[nodiscard]] constexpr bool requiresFlush() const noexcept{
			return index > 0;
		}

		virtual void post(const Texture* texture, float* vertices, int offset, int count) = 0;

		virtual void checkFlush(const Texture* texture, unsigned count) = 0;

		virtual void post(const std::function<void()>& drawPost){
			drawPost();
		}
	};

	struct [[jetbrains::guard]] BatchGuard{
		Batch& batch;

		[[nodiscard]] explicit BatchGuard(Batch& batch) noexcept
			: batch{batch}{}
	};

	/**
	 * @brief L2W --- Local Coordinate To World Transformation Matrix(2D)
	 */
	export struct [[jetbrains::guard]] BatchGuard_L2W : BatchGuard{
		const Geom::Matrix3D originalMat{};

		[[nodiscard]] explicit BatchGuard_L2W(Batch& batch, const Geom::Matrix3D& mat)
			: BatchGuard{batch}, originalMat{batch.getLocalToWorld()}{
			batch.flush();
			batch.setLocalToWorld(mat);
		}

		~BatchGuard_L2W(){
			batch.flush();
			batch.setLocalToWorld(originalMat);
		}
	};

	export struct [[jetbrains::guard]] BatchGuard_Blend : BatchGuard{
		const GL::Blending blending{};

		[[nodiscard]] explicit BatchGuard_Blend(Batch& batch, const GL::Blending blending)
			: BatchGuard{batch}, blending{blending}{
			batch.switchBlending(blending);
		}

		~BatchGuard_Blend(){
			batch.switchBlending(blending);
		}
	};

	/**
	 * @brief L2W --- Local Coordinate To World Transformation Matrix(2D)
	 */
	export struct [[jetbrains::guard]] BatchGuard_Proj : BatchGuard{
		Geom::Matrix3D current{};
		const Geom::Matrix3D* originalMat{};

		Geom::Matrix3D* operator ->(){
			return &current;
		}

		[[nodiscard]] explicit BatchGuard_Proj(Batch& batch, Concepts::Invokable<void(Geom::Matrix3D&)> auto&& modifier)
			: BatchGuard{batch}, originalMat{batch.getProjection()}{
			batch.flush();
			modifier(current);
			batch.setProjection(&current);
		}

		[[nodiscard]] explicit BatchGuard_Proj(Batch& batch)
			: BatchGuard{batch}, originalMat{batch.getProjection()}{
			batch.flush();
			batch.setProjection(&current);
		}

		[[nodiscard]] BatchGuard_Proj(Batch& batch, const Geom::Matrix3D& mat)
			: BatchGuard{batch}, current{mat}, originalMat{batch.getProjection()}{
			batch.flush();
			batch.setProjection(&current);
		}

		~BatchGuard_Proj(){
			batch.flush();
			batch.setProjection(originalMat);
		}
	};

	export struct [[jetbrains::guard]] BatchGuard_Shader : BatchGuard{
		GL::ShaderProgram* originalShader{nullptr};

		[[nodiscard]] explicit BatchGuard_Shader(Batch& batch, GL::ShaderProgram* shader) noexcept
			: BatchGuard{batch}, originalShader{batch.getCustomShader()}{
			batch.getGeneralShader()->bind();
			batch.getGeneralShader()->apply();
			batch.switchCustomShader(shader);
		}

		~BatchGuard_Shader(){
			batch.switchCustomShader(originalShader);
		}
	};
}


