module;

export module Math.Rand;

import std;
import ext.Concepts;

export namespace Math {
	/**
	 * Impl XorShift Random
	 * @author Inferno
	 * @author davebaol
	 */
	class Rand {
	public:
		using SeedType = std::size_t;

	private:
		/** Normalization constant for double. */
		static constexpr double NORM_DOUBLE = 1.0 / static_cast<double>(1ll << 53);
		/** Normalization constant for float. */
		static constexpr float NORM_FLOAT = 1.0f / static_cast<float>(1ll << 24);

		static constexpr SeedType murmurHash3(SeedType x) {
			x ^= x >> 33;
			x *= 0xff51afd7ed558ccdL;
			x ^= x >> 33;
			x *= 0xc4ceb9fe1a85ec53L;
			x ^= x >> 33;

			return x;
		}

		/** The first half of the internal state of this pseudo-random number generator. */
		SeedType seed0{};
		/** The second half of the internal state of this pseudo-random number generator. */
		SeedType seed1{};

		constexpr int next(const int bits) {
			return static_cast<int>(nextLong() & (1ull << bits) - 1ull);
		}

	public:
		constexpr Rand() { // NOLINT(*-use-equals-default)
			setSeed(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
		}

		/**
		 * @param seed the initial seed
		 */
		constexpr explicit Rand(const SeedType seed) {
			setSeed(seed);
		}

		/**
		 * @param seed0 the first part of the initial seed
		 * @param seed1 the second part of the initial seed
		 */
		constexpr Rand(const SeedType seed0, const SeedType seed1) {
			setState(seed0, seed1);
		}
		
		constexpr size_t nextLong() {
			SeedType s1       = this->seed0;
			const SeedType s0 = this->seed1;
			this->seed0       = s0;
			s1 ^= s1 << 23;
			return (this->seed1 = s1 ^ s0 ^ s1 >> 17ull ^ s0 >> 26ull) + s0;
		}

		/**
		 * Returns the next pseudo-random, uniformly distributed {int} value from this random number generator's sequence.
		 */
		constexpr int nextInt() {
			return static_cast<int>(nextLong());
		}

		/**
		 * @param n_exclusive the positive bound on the random number to be returned.
		 * @return the next pseudo-random {int} value between {0} (inclusive) and {n} (exclusive).
		 */
		constexpr int nextInt(const int n_exclusive) {
			return static_cast<int>(nextLong(n_exclusive));
		}

		/**
		 * Returns a pseudo-random, uniformly distributed {long} value between 0 (inclusive) and the specified value (exclusive),
		 * drawn from this random number generator's sequence. The algorithm used to generate the value guarantees that the result is
		 * uniform, provided that the sequence of 64-bit values produced by this generator is.
		 * @param n_exclusive the positive bound on the random number to be returned.
		 * @return the next pseudo-random {long} value between {0} (inclusive) and {n} (exclusive).
		 */
		constexpr size_t nextLong(const size_t n_exclusive) {
			for(;;) {
				const size_t bits  = nextLong() >> 1ull;
				const size_t value = bits % n_exclusive;
				if(bits >= value + (n_exclusive - 1)) return value;
			}
		}

		/**
		 * @return a pseudo-random, uniformly distributed {double} value between 0.0 and 1.0 from this random number generator's
		 * sequence.
		 */
		constexpr double nextDouble() {
			return static_cast<double>(nextLong() >> 11) * NORM_DOUBLE;
		}

		/**
		 * @return a pseudo-random, uniformly distributed {float} value between 0.0 and 1.0 from this random number generator's
		 * sequence.
		 */
		// ReSharper disable once CppDFAUnreachableFunctionCall
		constexpr float nextFloat() {
			return static_cast<float>(nextLong() >> 40) * NORM_FLOAT; // NOLINT(*-narrowing-conversions)
		}

		/**
		 * @return a pseudo-random, uniformly distributed {boolean } value from this random number generator's sequence.
		 */
		constexpr bool nextBoolean() {
			return (nextLong() & 1) != 0;
		}

		/**
		 * The given seed is passed twice through a hash function. This way, if the user passes a small value we avoid the short
		 * irregular transient associated with states having a very small number of bits set.
		 * @param _seed a nonzero seed for this generator (if zero, the generator will be seeded with @link std::numeric_limits<SeedType>::lowest() @endlink ).
		 */
		constexpr void setSeed(const SeedType _seed) {
			const SeedType seed0 = murmurHash3(_seed == 0 ? std::numeric_limits<SeedType>::lowest() : _seed);
			setState(seed0, murmurHash3(seed0));
		}

		constexpr bool chance(const double chance) {
			return nextDouble() < chance;
		}

		constexpr int range(const int amount) {
			return nextInt(amount * 2 + 1) - amount;
		}

		constexpr float range(const float amount) {
			return nextFloat() * amount * 2 - amount;
		}

		constexpr float random(const float max) {
			return nextFloat() * max;
		}

		/** Inclusive. */
		constexpr int random(const int max) {
			return nextInt(max + 1);
		}

		constexpr float random(const float min, const float max) {
			return min + (max - min) * nextFloat();
		}

		constexpr int random(const int min, const int max) {
			if(min >= max) return min;
			return min + nextInt(max - min + 1);
		}

		/**
		 * Sets the internal state of this generator.
		 * @param _seed0 the first part of the internal state
		 * @param _seed1 the second part of the internal state
		 */
		constexpr void setState(const SeedType _seed0, const SeedType _seed1) {
			this->seed0 = _seed0;
			this->seed1 = _seed1;
		}

		/**
		 * Returns the internal seeds to allow state saving.
		 * @param seedIndex must be 0 or 1, designating which of the 2 long seeds to return
		 * @return the internal seed that can be used in setState
		 */
		[[nodiscard]] constexpr SeedType getState(const int seedIndex) const {
			return seedIndex == 0 ? seed0 : seed1;
		}
	};

	thread_local Rand globalRand{};
}
