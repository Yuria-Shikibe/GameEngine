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
		bool focusWhenInbound{true};

		~Screen() override{
			endDraw_noContextFallback();
			endCameraFocus();
		}

		[[nodiscard]] Screen(){
			inputListener.on<UI::CurosrExbound>([this](const auto&) {
				if(focusWhenInbound)endCameraFocus();
			});

			inputListener.on<UI::CurosrInbound>([this](const auto&) {
				if(focusWhenInbound)beginCameraFocus();
			});

			inputListener.on<UI::MouseActionRelease>([this](const UI::MouseActionRelease& event) {
				if(event.key == Ctrl::Mouse::CMB){
					lastPos.setNaN();
				}
			});

			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				if(event.key == Ctrl::Mouse::CMB){
					if(lastPos.isNaN()){
						lastPos.set(camera.getPosition());
					}else{
						camera.setPosition(lastPos + event.getRelativeMove() * (-1 / camera.getScale()));
					}
				}
			});

			dropFocusAtCursorQuitBound = false;
			touchbility = TouchbilityFlags::enabled;
		}

		bool setWidth(const float w) noexcept override{
			if(Elem::setWidth(w)){
				buffer.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				camera.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				return true;
			}
			return false;
		}

		bool setHeight(const float h) noexcept override{
			if(Elem::setHeight(h)){
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

		void update(const float delta) override{
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

		[[nodiscard]] Core::Camera2D& getCamera() noexcept{ return camera; }
	};
}
