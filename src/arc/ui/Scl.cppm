module;

export module UI.Scl;

namespace UI {
	float width;
	float height;
	float ratio_WH;

	float defScale = 1.0f;
	float defRes_w = 1920;
	float defRes_h = 1080;

	float outputScale = defScale;
}

export namespace UI {
	void setScreenSize(const float w, const float h) {
		width = w;
		height = h;

		outputScale = w * h / (defRes_w * defRes_h) * defScale;

		ratio_WH = w / h;
	}

	float getRatio() {
		return ratio_WH;
	}

	float getSquareRatio_W2H(const float w) {
		return w / ratio_WH;
	}

	float getSquareRatio_H2W(const float h) {
		return h * ratio_WH;
	}

	float getRecommendedScale() {
		return outputScale;
	}
}
