//
// Created by Matrix on 2024/3/31.
//

export module UI.ProgressBar;

export import UI.Elem;
export import Math.Interpolation;

import std;

export namespace UI{
	struct ProgressBarDrawer;

	class ProgressBar : public Elem{
	public:
		enum struct ApproachScope : bool{
			drawing = false,
			updating = true
		};

	protected:
		const ProgressBarDrawer* barDrawer{};
		mutable float lastProgress{};

		ApproachScope approachScope = ApproachScope::drawing;

		Math::DiffApproach::DiffApproachFuncPtr
			approachFunc = Math::DiffApproach::ratioApproaching;

		void updateProgress(const float delta = 1.0f) const {
			if(approachFunc && progressGetter){
				lastProgress = approachFunc(lastProgress, progressGetter(), approachSpeed * delta);
			}else{
				lastProgress = 0;
				//TODO throw maybe?
			}
		}

	public:
		explicit ProgressBar(Root* root = nullptr) : Elem{root}{
			ProgressBar::applyDefDrawer();
		}

		const float approachSpeed = 0.0125f;
		std::function<float()> progressGetter;

		[[nodiscard]] float getDrawProgress() const{ return lastProgress; }


		void update(const Core::Tick delta) override{
			Elem::update(delta);

			if(approachScope == ApproachScope::updating){
				updateProgress(delta);
			}
		}

		void drawContent() const override;

		void applyDefDrawer() noexcept override;
	};

}