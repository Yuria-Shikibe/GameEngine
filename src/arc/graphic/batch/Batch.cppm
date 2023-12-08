//
// Created by Matrix on 2023/11/19.
//

module ;

export module Core.Batch;

import <memory>;
import <functional>;
import GL.Blending;
import GL.Constants;
import GL.Mesh;
import GL.Shader;
import GL.Texture.Texture2D;
import RuntimeException;

import Geom.Matrix3D;

using namespace GL;

export namespace Core{
	inline constexpr int VERT_GROUP_SIZE = 2 + 2 + 4 + 4;
	inline constexpr int VERT_GROUP_COUNT = 4;
	inline constexpr int VERT_LENGTH_STD = VERT_GROUP_COUNT * VERT_GROUP_SIZE;

	class Batch
	{
	protected:
		std::unique_ptr<Mesh> mesh = nullptr;

		const Texture2D* lastTexture = nullptr;

		Shader* generalShader = nullptr;
		Shader* customShader = nullptr;

		const Blending* blending = &Blendings::NORMAL;

		/**
		 * \brief From Viewport Size to normalized [-1, 1]
		 */
		const Geom::Matrix3D* projection = nullptr;

		// const Geom::Matrix3D*

		const Geom::Matrix3D* projectionFallback = nullptr;

		GLsizei index = 0;

	public:
		Batch() {
			blending->apply();
		}

		Batch(const Batch& other) = delete;

		Batch(Batch&& other) = delete;

		Batch& operator=(const Batch& other) = delete;

		Batch& operator=(Batch&& other) = delete;

		virtual void bindAll() const {
			mesh->bind();
		}

		[[nodiscard]] bool hasShader() const {
			return customShader || generalShader;
		}

		[[nodiscard]] bool applyCustomShader() const {
			return customShader;
		}

		[[nodiscard]] Shader& getGeneralShader() const {
			return *generalShader;
		}

		[[nodiscard]] const Texture2D* getTexture() const{
			return lastTexture;
		}

		void setCustomShader(Shader& shader) {
			customShader = &shader;
			customShader->bind();
		}

		void setCustomShader() {
			customShader = nullptr;
			generalShader->bind();
		}

		void switchShader(Shader& shader) {
			flush();

			setCustomShader(shader);
		}

		void clearCustomShader(const bool flushContext = true) {
			if(flushContext)flush();

			customShader = nullptr;
			generalShader->bind();
		}

		void switchBlending(const Blending& b){
			if(blending != &b){
				flush();
			}

			blending = &b;
			blending->apply();
		}

		[[nodiscard]] const Mesh* getMesh() const {
			return mesh.get();
		}

		void setProjection(const Geom::Matrix3D* porj){
			projection = porj;
		}

		void setProjection(const Geom::Matrix3D& porj){
			projection = &porj;
		}

		void beginProjection(const Geom::Matrix3D& porj) {
			if(&porj == projection || porj == *projection)return;

			if(projectionFallback != nullptr) {
				throw ext::IllegalArguments{"Cannot begin porjection before end the former one!"};
			}

			flush();

			projectionFallback = projection;
			projection = &porj;
		}

		void endProjection() {
			if(projectionFallback == nullptr)return;

			flush();

			projection = projectionFallback;
			projectionFallback = nullptr;
		}

		[[nodiscard]] const Geom::Matrix3D* getProjection() const{
			return projection;
		}

		void bindShader() const{
			if(applyCustomShader()){
				customShader->bind();
			}else{
				generalShader->bind();
			}
		}

		void applyShader() const{
			if(applyCustomShader()){
				customShader->apply();
			}else{
				generalShader->apply();
			}
		}

		virtual ~Batch() = default;

		virtual void flush() = 0;

		virtual void post(const Texture2D& texture, float* vertices, int length, int offset, int count) = 0;

		virtual void post(const std::function<void()>& drawPost){
			drawPost();
		}
	};
}


