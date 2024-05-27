//
// Created by Matrix on 2024/5/26.
//

export module Graphic.Draw:Mesh;


import std;
export import GL.Mesh;
export import GL.Shader;
export import GL.Constants;
export import ext.RuntimeException;

import :Frame;

export namespace Graphic::Mesh{
	inline std::stack<const GL::Mesh*> formerMesh{};

	inline void meshBegin(const GL::Mesh* mesh = nullptr){
		if(!mesh) mesh = Frame::rawMesh;
		mesh->bind();
		formerMesh.push(mesh);
	}

	//Use this for safety!
	inline void meshEnd(const GL::Mesh* const mesh, const bool render = false){
		if(render) mesh->render();
		if(mesh == formerMesh.top()){
			formerMesh.pop();

			if(!formerMesh.empty()) formerMesh.top()->bind();
		} else{
			throw ext::RuntimeException{"Cannot end incorredt mesh!"};
		}
	}

	inline void meshEnd(const bool render = false, const GL::ShaderProgram* shader = nullptr){
		if(render){
			if(shader){
				shader->bind();
				shader->apply();
			}
			formerMesh.top()->render(GL_TRIANGLE_FAN, 0, GL::ELEMENTS_QUAD_STRIP_LENGTH);
		}
		formerMesh.pop();

		if(!formerMesh.empty()) formerMesh.top()->bind();
	}
}

