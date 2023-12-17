module;

export module UI.Stage;

export import UI.Root;


export namespace UI {
	 class Stage : public Root {
	 public:
		 ~Stage() override = default;

		 void update(const float delta) override {
			 // this->root->updateChildren(delta);
		 }

		 void render() const override {
			// this->root->draw();
		 }
	 };
}