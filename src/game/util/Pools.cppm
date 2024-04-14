module;

export module Game.Pool;

import Concepts;
export import ext.Container.ObjectPoolGroup;
import ext.RuntimeException;
import std;

export namespace Game::Pools{
	ext::ObjectPoolGroup entityPoolGroup{};
}
