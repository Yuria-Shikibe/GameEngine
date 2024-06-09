//
// Created by Matrix on 2024/5/21.
//

export module UI.Screen;

export import UI.Elem;
import GL.Buffer.FrameBuffer;
import Core.Camera;
import Core.BatchGroup;
import std;
import Geom.Matrix3D;


export namespace UI{
	class Screen : public Elem{
		Core::Camera2D camera{};
		GL::FrameBuffer buffer{GL::FrameBuffer::RecommendedMinSize, GL::FrameBuffer::RecommendedMinSize, 2};

		mutable const Geom::Matrix3D* fallbackMat{};
		mutable std::unique_ptr<Core::Batch> Core::BatchGroup::* batchPtr{};

		Geom::Vec2 lastPos{Geom::SNAN2};

		void endDraw_noContextFallback() const;
	public:
		int requiredMode = Ctrl::Mode::Ignore;
		bool focusWhenInbound{true};
		/** @brief Whether adapt ui bloom effect*/
		bool usesUIEffect{true};

		~Screen() override{
			endDraw_noContextFallback();
			endCameraFocus();
		}

		[[nodiscard]] Screen(){
			inputListener.on<CurosrExbound>([this](const auto&) {
				if(focusWhenInbound)endCameraFocus();
			});

			inputListener.on<CurosrInbound>([this](const auto&) {
				if(focusWhenInbound)beginCameraFocus();
			});

			inputListener.on<MouseActionDoubleClick>([this](const MouseActionDoubleClick& event) {
				if(event.key == Ctrl::Mouse::CMB){
					camera.setPosition(camera.getMouseToWorld(Geom::Vector2D(event), absoluteSrc));
				}
			});

			inputListener.on<MouseActionRelease>([this](const MouseActionRelease& event) {
				if(event.key == Ctrl::Mouse::CMB){
					lastPos.setNaN();
				}
			});

			inputListener.on<MouseActionDrag>([this](const MouseActionDrag& event) {
				if(event.key == Ctrl::Mouse::CMB){
					if(Ctrl::Mode::modeMatch(event.mode, requiredMode)){
						if(lastPos.isNaN()){
							lastPos.set(camera.getPosition());
						}else{
							camera.setPosition(lastPos + event.getRelativeMove() * (-1 / camera.getScale()));
						}
					}else{
						lastPos.setNaN();
					}
				}
			});

			dropFocusAtCursorQuitBound = false;
			touchbility = TouchbilityFlags::enabled;
		}

		bool setWidth_Quiet(const float w) noexcept override{
			if(Elem::setWidth_Quiet(w)){
				buffer.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				camera.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				return true;
			}
			return false;
		}

		bool setHeight_Quiet(const float h) noexcept override{
			if(Elem::setHeight_Quiet(h)){
				buffer.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				camera.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				return true;
			}
			return false;
		}

		void draw() const override;

		void beginDraw(std::unique_ptr<Core::Batch> Core::BatchGroup::* batchPtr = nullptr) const;

		void endDraw() const;

		void drawContent() const override;

		void update(const Core::Tick delta) override{
			Elem::update(delta);
			camera.update(delta);
		}

		CursorType getCursorType() const noexcept override{
			if(lastPos.isNaN()){
				return CursorType::select_regular;
			}else{
				return CursorType::drag;
			}
		}

		void beginCameraFocus();
		void endCameraFocus() const;

		void lockViewport() const noexcept;
		void unlockViewport() const noexcept;

		Core::Batch* getBatch() const;

		[[nodiscard]] Core::Camera2D& getCamera() noexcept{ return camera; }

		[[nodiscard]] const GL::FrameBuffer& getBuffer() const{ return buffer; }

		[[nodiscard]] Geom::Vec2 getCursorPosInScreen() const{
			return camera.getMouseToWorld(getCursorPos(), absoluteSrc);
		}
	};
}
