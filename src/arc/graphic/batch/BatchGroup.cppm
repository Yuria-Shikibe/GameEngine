//
// Created by Matrix on 2024/3/19.
//

export module Core.BatchGroup;

export import Core.Batch;

import std;

export namespace Core{
	struct BatchGroup {
		std::unique_ptr<Batch> overlay{nullptr};
		std::unique_ptr<Batch> world{nullptr};

		void flushAll() const{
			if(overlay)overlay->flush();
			if(world)world->flush();
		}
	};
}
