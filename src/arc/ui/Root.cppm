module;

export module UI.Root;

import <memory>;
import <vector>;
import Container.Pool;

import Geom.Matrix3D;

export import UI.Elem;
export import UI.Table;

import Graphic.Resizeable;
import OS.ApplicationListener;

export namespace UI{

class Root : public Graphic::Resizeable<unsigned int>, public OS::ApplicationListener{
	protected:
	Geom::Matrix3D projection{};
	public:
		virtual ~Root() = default;

		[[nodiscard]] Root() : root(std::make_unique<Table>()) {
			root->setSrc(0.0f, 0.0f);
			root->getAbsSrc().setZero();
			root->relativeLayoutFormat = false;
		}

	//
		Elem* currentInputFocused{nullptr};
		// // //Focus
		// //
		// // //Input Listeners
		std::unique_ptr<Table> root{nullptr};

		//
		// //Renderers
		//
		void update(const float delta) override {
			root->update(delta);
		}

		[[nodiscard]] Geom::Matrix3D& getPorj() {
			return projection;
		}

		void resize(const unsigned w, const unsigned h) override {
			root->setSize(static_cast<float>(w), static_cast<float>(h));
			constexpr float margin = 8.0f;
			projection.setOrthogonal(-margin, -margin, static_cast<float>(w) + margin * 2.0f, static_cast<float>(h) + margin * 2.0f);
		}

		virtual void render() const {
			root->draw();
		}
	};
}
