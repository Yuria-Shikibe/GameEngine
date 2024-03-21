//
// Created by Matrix on 2024/3/19.
//

export module Core.BatchGroup;

export import Core.Batch;

import std;

export namespace Core{
	struct BatchGroup {
		std::unique_ptr<Batch> batchOverlay{nullptr};
		std::unique_ptr<Batch> batchWorld{nullptr};
	};
}
