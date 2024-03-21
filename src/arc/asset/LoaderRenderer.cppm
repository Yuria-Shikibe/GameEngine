module;

#include <glad/glad.h>

export module Assets.LoaderRenderer;

import Align;

import Core.Renderer;

import Graphic.Draw;
import Graphic.Color;

import Geom.Matrix3D;

import Core;
import GL.Buffer.FrameBuffer;
import Assets.Loader;
import Assets.Graphic;

import Font.GlyphArrangement;

import <GLFW/glfw3.h>;
import std;
import GL;

using namespace Graphic;

export namespace Assets {
	class LoaderRenderer final : public Core::Renderer{
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
			: Renderer(w, h), loader(loader), drawFBO(w, h) {

			mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));

			defaultMat = Graphic::Batch::getPorj();
			Graphic::Batch::beginPorj(mat);

			GL::setStencilOperation(GL::Operation::KEEP, GL::Operation::KEEP, GL::Operation::REPLACE);

			lastThreshold = Assets::PostProcessors::bloom->threshold;
			Assets::PostProcessors::bloom->threshold = 0.0f;
		}

		~LoaderRenderer() override { // NOLINT(*-use-equals-default)
			Graphic::Batch::beginPorj(mat);
			Graphic::Batch::shader(false);

			GL::bindFrameBuffer(GL_FRAMEBUFFER);
			GL::setStencilOperation(GL::Operation::REPLACE, GL::Operation::REPLACE, GL::Operation::REPLACE);
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

			Graphic::Batch::shader();

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
			Draw::Line::setLineStroke((stroke + slideLineSize) * 2.0f);
			Draw::Line::line(0, y, w, y);


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

			Graphic::Batch::shader(Assets::Shaders::sildeLines, true);
			Draw::Line::setLineStroke(stroke);
			Draw::color(Colors::GRAY);

			Graphic::Batch::flush();
			//Begin Mask Draw
			GL::enable(GL::Test::STENCIL);
			GL::setStencilFunc(GL::Func::ALWAYS, 0xFF, 0xFF);
			GL::setStencilMask(0xFF);

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

			//End Mask Draw
			Graphic::Batch::flush();
			GL::setStencilFunc(GL::Func::EQUAL, 0xFF, 0xFF);
			GL::setStencilMask(0x00);

			Draw::Line::setLineStroke(stroke * 4);
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

			Graphic::Batch::flush();
			GL::disable(GL::Test::STENCIL);
			//End Clip Layer Draw
			Graphic::Batch::shader();

			ss.str("");
			ss << "${scl#[0.4]}Loading${scl#[0.3]}: (${color#[" << end << "]}"<< std::fixed << std::setprecision(1) << lastProgress * 100.0f << "${scl#[0.25]}%${color#[]}${scl#[0.3]})";
			ss << "\n${scl#[0.3]}${color#[" << end << "]}" << static_cast<float>(loader->getTimer().toMark().count()) / 1000.0f << "${color#[]}sec.";

			Font::glyphParser->parse(loadStatus, ss.str());
			loadStatus->offset.set(x, y - stroke - slideLineSize * 2.0f);

			loadStatus->setAlign(Align::Mode::top_left);
			loadStatus->render();
		}

		void draw() override {
			defaultFrameBuffer.clearColor();

			drawFBO.clearColor();
			drawFBO.clearRenderData();
			drawFBO.bind();
			glClear(GL_STENCIL_BUFFER_BIT);

			drawMain();

			const auto times = Assets::PostProcessors::bloom->blur.getProcessTimes();
			Assets::PostProcessors::bloom->blur.setProcessTimes(4);
			Assets::PostProcessors::bloom->setIntensity(1.0f);

			Assets::PostProcessors::bloom->apply(&drawFBO, &defaultFrameBuffer);

			Assets::PostProcessors::bloom->blur.setProcessTimes(times);
			Assets::PostProcessors::bloom->setIntensity(1.1f);

			glBlitNamedFramebuffer(defaultFrameBuffer.getID(), 0, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		void resize(const unsigned w, const unsigned h) override {
			drawFBO.resize(w, h);

			mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
		}
	};
}