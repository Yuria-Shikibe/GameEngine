module;

#include <glad/glad.h>

export module Assets.LoaderRenderer;

import UI.Align;

import Core.Renderer;

import Graphic.Draw;
import Graphic.Color;

import Geom.Matrix3D;

import Core;
import GL.Buffer.FrameBuffer;
import Assets.Loader;
import Assets.Graphic;

import Font.GlyphArrangement;

import std;
import GL;
import ext.Guard;

export namespace Assets {
	class LoaderRenderer final : public Core::Renderer{
		AssetsLoader* loader{nullptr};

		Geom::Matrix3D mat{};

		const std::shared_ptr<Font::GlyphLayout> loadStatus = Font::obtainLayoutPtr();
		const std::shared_ptr<Font::GlyphLayout> loadTasks = Font::obtainLayoutPtr();

		std::stringstream ss{};

	public:
		//TODO remove these to settings
		float lastProgress = 0.0f;
		float lastThreshold = 0.0f;

		[[nodiscard]] LoaderRenderer(const int w, const int h, AssetsLoader* const loader)
			: Renderer(w, h), loader(loader) {
			GL::setStencilOperation(GL::StencilOperation::KEEP, GL::StencilOperation::KEEP, GL::StencilOperation::REPLACE);

			lastThreshold = PostProcessors::bloom->threshold;
			PostProcessors::bloom->threshold = 0.0f;

			mat.setOrthogonal(0.f, 0.f, getDrawWidth(), getDrawHeight());
		}

		~LoaderRenderer() override { // NOLINT(*-use-equals-default)
			//maybe should be set after load done
			//Core::batchGroup.overlay->setProjection(defaultMat);
			Graphic::Draw::Overlay::getBatch().clearCustomShader(false);

			GL::bindFrameBuffer(GL::FrameBuffer::DEF, 0);
			GL::setStencilOperation(GL::StencilOperation::REPLACE, GL::StencilOperation::REPLACE, GL::StencilOperation::REPLACE);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			PostProcessors::bloom->threshold = lastThreshold;
		}

		void drawMain() override {
			using namespace Graphic;

			constexpr float stroke = 10.0f;
			const float y = static_cast<float>(getHeight()) * 0.5f;
			const auto w = static_cast<float>(getWidth());
			const float x = w * 0.25f;
			const float barWidth = w * 0.5f;

			constexpr float slideLineSize = 12.0f;
			constexpr float preBlockWidth = 32.0f;

			using Draw::Overlay;

			[[maybe_unused]] Core::BatchGuard_Proj guard_proj{Overlay::getBatch(), mat};

			Overlay::mixColor(Colors::DARK_GRAY);

			if(!loader->finished()) {
				Font::defGlyphParser->parseWith(loadTasks, loader->getTaskNames("$<alp#[0.3]>$<scl#[1.5]>", ">> "));
				lastProgress = std::lerp(lastProgress, loader->getProgress(), 0.075f);
			}else {
				Font::defGlyphParser->parseWith(loadTasks, "$<alp#[0.3]>$<scl#[1.8]>LOAD DONE");
				lastProgress = std::lerp(lastProgress, loader->getProgress(), 0.15f);
			}

			loadTasks->setAlign(Align::Mode::bottom_left);
			loadTasks->offset.set(w * 0.05f, y * 0.1f);
			loadTasks->render();

			Overlay::mixColor();

			Overlay::color(Colors::DARK_GRAY);

			Overlay::alpha(0.177f);
			Overlay::Line::setLineStroke((stroke + slideLineSize) * 2.0f);
			Overlay::Line::line(0, y, w, y);


			Overlay::alpha();
			Overlay::Fill::quad(
				x - preBlockWidth, y - stroke - slideLineSize,
				x + slideLineSize, y - stroke - slideLineSize,
				x + slideLineSize, y + stroke,
				x - preBlockWidth, y + stroke
			);

			Overlay::Fill::quad(
				x - preBlockWidth, y + stroke,
				x + slideLineSize, y + stroke,
				x + slideLineSize, y + stroke + slideLineSize,
				x - preBlockWidth + slideLineSize, y + stroke + slideLineSize
			);

			const Color begin = loader->postedTasks.empty() ? Colors::ROYAL : Colors::RED;
			const Color end = begin.createLerp(loader->postedTasks.empty() ? Colors::SKY : Colors::ORANGE, lastProgress);

			{
				Overlay::getBatch().flush();

				[[maybe_unused]] GL::StateGurad<GL::Test::STENCIL> stateGurad{};
				[[maybe_unused]] GL::UniformGuard guard{Shaders::slideLineShaderAngle, -135.0f};
				[[maybe_unused]] Core::BatchGuard_Shader guard_shader{Overlay::getBatch(), Shaders::sildeLines};

				[[maybe_unused]] ext::GuardRef guard1{Overlay::Line::contextStroke, stroke};
				[[maybe_unused]] ext::GuardRef guard2{Overlay::contextColor, Colors::GRAY};

				//Begin Mask Draw
				GL::setStencilMask(0xFF);
				GL::setStencilFunc(GL::Func::ALWAYS, 0xFF, 0xFF);

				Overlay::Fill::quad(
					Overlay::getDefaultTexture(),
					x, y - stroke - slideLineSize, Colors::DARK_GRAY,
					x + barWidth - slideLineSize, y - stroke - slideLineSize, Colors::GRAY,
					x + barWidth, y - stroke, Colors::GRAY,
					x, y + stroke, Colors::DARK_GRAY
				);

				Overlay::Fill::quad(
					Overlay::getDefaultTexture(),
					x, y + stroke, Colors::DARK_GRAY,
					x + barWidth, y - stroke, Colors::GRAY,
					x + barWidth, y + stroke + slideLineSize, Colors::GRAY,
					x + slideLineSize, y + stroke + slideLineSize, Colors::DARK_GRAY
				);

				//End Mask Draw
				Overlay::getBatch().flush();

				GL::setStencilFunc(GL::Func::EQUAL, 0xFF, 0xFF);
				GL::setStencilMask(0x00);

				Overlay::Line::setLineStroke(stroke * 4);
				Overlay::alpha(0.9f);

				Overlay::Fill::quad(
					Overlay::getDefaultTexture(),
					x, y - stroke, begin,
					x + barWidth * lastProgress, y - stroke, end,
					x + barWidth * lastProgress, y + stroke + slideLineSize, end,
					x, y + stroke + slideLineSize, begin
				);

				Overlay::Fill::quad(
					Overlay::getDefaultTexture(),
					x, y - stroke - slideLineSize, begin,
					x + barWidth * lastProgress - slideLineSize, y - stroke - slideLineSize, end,
					x + barWidth * lastProgress, y - stroke, end,
					x, y - stroke, begin
				);

				GL::setStencilMask(0xFF);
			}

			ss.str("");
			ss << "$<scl#[0.4]>Loading$<scl#[0.3]>: ($<color#[" << end << "]>";
			ss << std::fixed << std::setprecision(1) << lastProgress * 100.0f << "$<scl#[0.25]>%$<color#[]>$<scl#[0.3]>)";
			ss << "\n$<scl#[0.3]>$<color#[" << end << "]>" << static_cast<float>(loader->getTimer().toMark().count()) / 1000.0f << "$<color#[]>sec.";

			Font::defGlyphParser->parseWith(loadStatus, std::move(ss).str());
			loadStatus->offset.set(x, y - stroke - slideLineSize * 2.0f);

			loadStatus->setAlign(Align::Mode::top_left);
			loadStatus->render();
		}

		void draw() override {
			defaultFrameBuffer.clearColor();

			effectBuffer.clearColor();
			effectBuffer.clearRenderData();
			effectBuffer.bind();
			effectBuffer.clearStencil();

			GL::viewport(width, height);
			drawMain();

			const auto times = PostProcessors::bloom->blur.getProcessTimes();
			PostProcessors::bloom->blur.setProcessTimes(2);
			PostProcessors::bloom->setIntensity(1.f, 1.f);

			PostProcessors::bloom->apply(&effectBuffer, &defaultFrameBuffer);

			PostProcessors::bloom->blur.setProcessTimes(times);

			GL::bindFrameBuffer(GL::FrameBuffer::READ, defaultFrameBuffer.getID());
			GL::bindFrameBuffer(GL::FrameBuffer::DRAW, 0);

			GL::blit(0, 0, width, height,
				0, 0, width, height,
				GL_COLOR_BUFFER_BIT, GL::nearest);
		}

		void resize(const int w, const int h) override {
			mat.setOrthogonal(0.0f, 0.0f, getDrawWidth(), getDrawHeight());

			Renderer::resize(w, h);
		}
	};
}