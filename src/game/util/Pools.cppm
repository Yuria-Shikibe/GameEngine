module;

export module Game.Pool;

import ext.Concepts;
export import ext.Container.ObjectPoolGroup;
import ext.RuntimeException;
import std;

export namespace Game::Pools{
	ext::ObjectPoolGroup entityPoolGroup{};
}
