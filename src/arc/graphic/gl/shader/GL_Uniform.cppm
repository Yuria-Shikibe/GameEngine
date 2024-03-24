module;

#include <glad/glad.h>

export module GL.Uniform;

import Graphic.Color;
import Geom.Matrix3D;

using namespace Graphic;

export namespace GL{
	inline void uniformColor(const GLint location, const Color& color){
		glUniform4f(location, color.r, color.g, color.b, color.a);
	}

	inline void uniformTexture(const GLint location, const GLint& texSlot) {
		glUniform1i(location, texSlot);
	}

	inline void uniform1i(const GLint location, const GLint& unit){
		glUniform1i(location, unit);
	}

	inline void uniformMat3D(const GLint location, const Geom::Matrix3D& mat){
		glUniformMatrix3fv(location, 1, GL_FALSE, mat.getRawVal());
	}
}
