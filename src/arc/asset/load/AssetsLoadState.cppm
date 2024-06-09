//
// Created by Matrix on 2024/6/8.
//

export module Assets.Load.State;

import Core.Async.TaskQueue;
import ext.Event;
import std;

export namespace Assets::Load{
	enum struct Phase : unsigned{
		init, //Task init
		pull, //Pull & Process Request
		load, //Texture & Audio Load
		post_load, //Load Done - Post Process
		end, //All Load Done
		clear //Load Resource Clear
	};

	using LoadEventManager = ext::SignalManager<Phase, std::to_underlying(Phase::clear) + 1>;

	enum struct State{
		running,
		blocked,
		finished,
		interrupted
	};
}