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

export namespace Core{
	class Batch
	{
	protected:
		std::unique_ptr<Mesh> mesh = nullptr;

		const Texture* lastTexture = nullptr;

		Shader* generalShader = nullptr;
		Shader* customShader = nullptr;

		const Blending* blending = &Blendings::Normal;

		/**
		 * \brief From Viewport Size to normalized [-1, 1]
		 */
		const Geom::Matrix3D* projection = nullptr;

		// const Geom::Matrix3D*

		const Geom::Matrix3D* projectionFallback = nullptr;

		unsigned index = 0;

	public:
		Batch() {
			setupBlending();
		}

		Batch(const Batch& other) = delete;

		Batch(Batch&& other) = delete;

		Batch& operator=(const Batch& other) = delete;

		Batch& operator=(Batch&& other) = delete;

		virtual void bindAll() const {
			mesh->bind();
		}

		virtual void reset(){
			projectionFallback = nullptr;
			lastTexture = nullptr;
		}

		[[nodiscard]] bool hasShader() const {
			return customShader || generalShader;
		}

		[[nodiscard]] bool applyCustomShader() const {
			return customShader;
		}

		[[nodiscard]] Shader* getGeneralShader() const {
			return generalShader;
		}

		[[nodiscard]] const Texture* getTexture() const{
			return lastTexture;
		}

		void setCustomShader(Shader* shader) {
			customShader = shader;
		}

		void setCustomShader() {
			customShader = nullptr;
		}

		void setupBlending() const {
			blending->apply();
		}

		void switchShader(Shader* shader) {
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
			if(applyCustomShader()){
				customShader->bind();
			}else{
				generalShader->bind();
			}
		}

		void applyShader() const{
			if(applyCustomShader()){
				//TODO this is really dangerous!
				customShader->applyDynamic(generalShader->getDrawer(), true);
			}else{
				generalShader->apply();
			}
		}

		virtual ~Batch() = default;

		virtual void flush() = 0;

		virtual void post(const Texture* texture, float* vertices, int offset, int count) = 0;

		virtual void post(const std::function<void()>& drawPost){
			drawPost();
		}
	};
}


