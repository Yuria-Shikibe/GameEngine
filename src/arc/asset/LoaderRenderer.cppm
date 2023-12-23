module;

export module Assets.LoaderRenderer;

import Align;

import Graphic.RendererImpl;

import Graphic.Draw;
import Graphic.Color;

import Geom.Matrix3D;

import Core;

import GL.Constants;
import GL.Buffer.FrameBuffer;
import Assets.Loader;
import Assets.Graphic;

import GlyphArrangement;

import <glad/glad.h>;
import <GLFW/glfw3.h>;
import <memory>;
import <iomanip>;
import <sstream>;
import GL;

using namespace Graphic;

export namespace Assets {
	class LoaderRenderer final : public Graphic::RendererImpl {
		Assets::AssetsLoader* loader{nullptr};
		GL::FrameBuffer drawFBO{};
		Geom::Matrix3D mat{};
		const Geom::Matrix3D* defaultMat{nullptr};

		const std::shared_ptr<Font::GlyphLayout> loadStatus = Font::obtainLayoutPtr();
		const std::shared_ptr<Font::GlyphLayout> loadTasks = Font::obtainLayoutPtr();

		std::stringstream ss{};

	public:
		float lastProgress = 0.0f;
		float lastThreshold = 0.0f;

		[[nodiscard]] LoaderRenderer(const unsigned int w, const unsigned int h, Assets::AssetsLoader* const loader)
			: RendererImpl(w, h), loader(loader), drawFBO(w, h) {

			mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));

			defaultMat = Core::batch->getProjection();
			Core::batch->setProjection(mat);

			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			lastThreshold = Assets::PostProcessors::bloom->threshold;
			Assets::PostProcessors::bloom->threshold = 0.0f;
		}

		~LoaderRenderer() override { // NOLINT(*-use-equals-default)
			Core::batch->setProjection(defaultMat);
			Draw::shader(false);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			Assets::PostProcessors::bloom->threshold = lastThreshold;
		}

		void drawMain() override {
			constexpr float stroke = 10.0f;
			const float y = static_cast<float>(getHeight()) * 0.5f;
			const auto w = static_cast<float>(getWidth());
			const float x = w * 0.25f;
			const float barWidth = w * 0.5f;

			constexpr float slideLineSize = 12.0f;
			constexpr float preBlockWidth = 32.0f;

			Draw::shader();
			glStencilMask(0x00);

			Draw::mixColor(Colors::DARK_GRAY);

			if(!loader->finished()) {
				Font::glyphParser->parse(loadTasks, loader->getTaskNames("${alp#[0.3]}${scl#[1.5]}", ">> "));
				lastProgress = std::lerp(lastProgress, loader->getProgress(), 0.075f);
			}else {
				Font::glyphParser->parse(loadTasks, "${alp#[0.3]}${scl#[1.8]}LOAD DONE");
				lastProgress = std::lerp(lastProgress, loader->getProgress(), 0.15f);
			}

			loadTasks->setAlign(Align::Mode::bottom_left);
			loadTasks->offset.set(w * 0.05f, y * 0.1f);
			loadTasks->render();

			Draw::mixColor();

			Draw::color(Colors::DARK_GRAY);

			Draw::alpha(0.177f);
			Draw::setLineStroke((stroke + slideLineSize) * 2.0f);
			Draw::line(0, y, w, y);


			Draw::alpha();
			Draw::quad(
				x - preBlockWidth, y - stroke - slideLineSize,
				x + slideLineSize, y - stroke - slideLineSize,
				x + slideLineSize, y + stroke,
				x - preBlockWidth, y + stroke
			);

			Draw::quad(
				x - preBlockWidth, y + stroke,
				x + slideLineSize, y + stroke,
				x + slideLineSize, y + stroke + slideLineSize,
				x - preBlockWidth + slideLineSize, y + stroke + slideLineSize
			);

			Draw::shader(Assets::Shaders::sildeLines, true);
			Draw::setLineStroke(stroke);
			Draw::color(Colors::GRAY);

			Draw::flush();

			GL::enable(GL_STENCIL_TEST);

			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);

			Draw::quad(
				Draw::defaultTexture,
				x, y - stroke - slideLineSize, Colors::DARK_GRAY,
				x + barWidth - slideLineSize, y - stroke - slideLineSize, Colors::GRAY,
				x + barWidth, y - stroke, Colors::GRAY,
				x, y + stroke, Colors::DARK_GRAY
			);

			Draw::quad(
				Draw::defaultTexture,
				x, y + stroke, Colors::DARK_GRAY,
				x + barWidth, y - stroke, Colors::GRAY,
				x + barWidth, y + stroke + slideLineSize, Colors::GRAY,
				x + slideLineSize, y + stroke + slideLineSize, Colors::DARK_GRAY
			);

			// Draw::shader();
			Draw::flush();

			glStencilFunc(GL_EQUAL, 1, 0xFF);
			glStencilMask(0x00);

			Draw::setLineStroke(stroke * 4);
			Draw::alpha(0.9f);

			const Color& begin = loader->postedTasks.empty() ? Colors::ROYAL : Colors::RED;
			Color end = begin;
			end.lerp(loader->postedTasks.empty() ? Colors::SKY : Colors::ORANGE, lastProgress);

			Draw::quad(
				Draw::defaultTexture,
				x, y - stroke, begin,
				x + barWidth * lastProgress, y - stroke, end,
				x + barWidth * lastProgress, y + stroke + slideLineSize, end,
				x, y + stroke + slideLineSize, begin
			);

			Draw::quad(
				Draw::defaultTexture,
				x, y - stroke - slideLineSize, begin,
				x + barWidth * lastProgress - slideLineSize, y - stroke - slideLineSize, end,
				x + barWidth * lastProgress, y - stroke, end,
				x, y - stroke, begin
			);

			Draw::flush();
			GL::disable(GL_STENCIL_TEST);

			Draw::shader();

			ss.str("");
			ss << "${scl#[0.4]}Loading${scl#[0.3]}: (${color#[" << end << "]}"<< std::fixed << std::setprecision(1) << lastProgress * 100.0f << "${scl#[0.25]}%${color#[]}${scl#[0.3]})";
			ss << "\n${scl#[0.3]}${color#[" << end << "]}" << static_cast<float>(loader->getTimer().toMark().count()) / 1000.0f << "${color#[]}sec.";

			Font::glyphParser->parse(loadStatus, ss.str());
			loadStatus->offset.set(x, y - stroke - slideLineSize * 2.0f);

			loadStatus->setAlign(Align::Mode::top_left);
			loadStatus->render();
		}

		void draw() override {
			defaultFrameBuffer->bind();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			drawFBO.bind();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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