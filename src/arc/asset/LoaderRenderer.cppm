module;

#include <iomanip>

export module Assets.LoaderRenderer;

import Graphic.RendererImpl;

import Graphic.Draw;
import Graphic.Color;

import Geom.Matrix3D;

import Core;

import GL.Constants;
import GL.Buffer.FrameBuffer;
import Assets.Loader;
import Assets;

import GlyphArrangement;

import TimeMark;

import <glad/glad.h>;
import <memory>;
import <sstream>;
import GL;

using namespace Graphic;

export namespace Assets {
	class LoaderRenderer final : public Graphic::RendererImpl {
		Assets::AssetsLoader* loader{nullptr};
		GL::FrameBuffer drawFBO{};
		Geom::Matrix3D mat{};
		const Geom::Matrix3D* defaultMat{nullptr};

		const std::shared_ptr<Font::GlyphLayout> coordCenter = std::make_shared<Font::GlyphLayout>();
		std::stringstream ss{};

		ext::Timestamper interval{};

	public:
		float lastProgress = 0.0f;

		[[nodiscard]] LoaderRenderer(const unsigned int w, const unsigned int h, Assets::AssetsLoader* const loader)
			: RendererImpl(w, h), loader(loader), drawFBO(w, h) {

			mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));

			defaultMat = Core::batch->getProjection();
			Core::batch->setProjection(mat);

			interval.mark();
		}

		~LoaderRenderer() override { // NOLINT(*-use-equals-default)
			Core::batch->setProjection(defaultMat);
			Draw::shader(false);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		void drawMain() override {
			lastProgress = std::lerp(lastProgress, loader->getProgress(), 0.075f);

			constexpr float stroke = 15.0f;
			const float y = static_cast<float>(getHeight()) * 0.5f;
			const auto w = static_cast<float>(getWidth());
			const float x = w * 0.25f;
			const float barWidth = w * 0.5f;

			Draw::shader();

			Draw::color(Colors::WHITE);
			Draw::line(x - 7, y, x, y);

			ss.str("");
			ss << "${scl#[0.72]}(" << std::fixed << std::setprecision(2) << lastProgress << "${scl#[0.65]}%${scl#[0.72]})";
			ss << "\n${scl#[0.62]}" << static_cast<float>(interval.toMark().count()) / 1000.0f << "sec.";

			Font::glyphParser->parse(coordCenter, ss.str());
			coordCenter->offset.set(x, y - stroke);

			coordCenter->setAlign(Font::TypeSettingAlign::top_left);
			coordCenter->render();

			Draw::shader(Assets::Shaders::sildeLines, true);
			Draw::setLineStroke(stroke);
			Draw::color(Colors::GRAY);
			Draw::line(x, y, x + barWidth, y);

			Draw::color(Colors::AQUA, Colors::SKY, 0.5f);
			Draw::line(x, y, x + barWidth * lastProgress, y);

			Draw::flush();
		}


		void draw() override {
			defaultFrameBuffer->bind();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			drawFBO.bind();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			drawMain();

			Assets::PostProcessors::bloom->apply(&drawFBO, defaultFrameBuffer.get());

			glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFrameBuffer->getID());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		void resize(const unsigned w, const unsigned h) override {
			drawFBO.resize(w, h);

			mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
		}
	};
}