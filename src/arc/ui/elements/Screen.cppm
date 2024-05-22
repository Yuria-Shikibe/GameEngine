//
// Created by Matrix on 2024/5/21.
//

export module UI.Screen;

export import UI.Widget;
import GL.Buffer.FrameBuffer;
import Core.Camera;
import Core.BatchGroup;
import std;
import Geom.Matrix3D;


export namespace UI{
	class Screen : public Widget{
		Core::Camera2D camera{};
		GL::FrameBuffer buffer{GL::FrameBuffer::RecommendedMinSize, GL::FrameBuffer::RecommendedMinSize, 2};

		mutable const Geom::Matrix3D* fallbackMat{};
		mutable std::unique_ptr<Core::Batch> Core::BatchGroup::* batchPtr{};

		void endDraw_noContextFallback() const;
	public:
		bool focusWhenInbound{true};

		~Screen() override{
			endDraw_noContextFallback();
			endCameraFocus();
		}

		[[nodiscard]] Screen(){
			inputListener.on<UI::CurosrExbound>([this](const auto& event) {
				if(focusWhenInbound)endCameraFocus();
			});

			inputListener.on<UI::CurosrInbound>([this](const auto& event) {
				if(focusWhenInbound)beginCameraFocus();
			});

			touchbility = TouchbilityFlags::enabled;
		}

		bool setWidth(const float w) override{
			const bool changed = Widget::setWidth(w);
			if(changed){
				buffer.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				camera.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
			}
			return changed;
		}

		bool setHeight(const float h) override{
			const bool changed = Widget::setHeight(h);
			if(changed){
				buffer.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
				camera.resize(static_cast<int>(bound.getWidth()), static_cast<int>(bound.getHeight()));
			}
			return changed;
		}

		void draw() const override;

		void beginDraw(std::unique_ptr<Core::Batch> Core::BatchGroup::* batchPtr = nullptr) const;

		void endDraw() const;

		void drawContent() const override;

		void update(const float delta) override{
			Widget::update(delta);
			camera.update(delta);
		}

		void beginCameraFocus();
		void endCameraFocus() const;

		[[nodiscard]] Core::Camera2D& getCamera(){ return camera; }
	};
}
