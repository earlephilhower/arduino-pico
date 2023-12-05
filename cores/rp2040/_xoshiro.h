//----------------------------------------------------------------------------------------
//
//	Xoshiro-cpp
//	Xoshiro PRNG wrapper library for C++17 / C++20
//
//	Copyright (C) 2020 Ryo Suzuki <reputeless@gmail.com>
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files(the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions :
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//	
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
//----------------------------------------------------------------------------------------

# pragma once
# include <cstdint>
# include <array>
# include <limits>
# include <type_traits>
# if __has_cpp_attribute(nodiscard) >= 201907L
#	define XOSHIROCPP_NODISCARD_CXX20 [[nodiscard]]
# else
#	define XOSHIROCPP_NODISCARD_CXX20
# endif

namespace XoshiroCpp
{
	// A default seed value for the generators
	inline constexpr std::uint64_t DefaultSeed = 1234567890ULL;

	// Converts given uint32 value `i` into a 32-bit floating
	// point value in the range of [0.0f, 1.0f)
	template <class Uint32, std::enable_if_t<std::is_same_v<Uint32, std::uint32_t>>* = nullptr>
	[[nodiscard]]
	inline constexpr float FloatFromBits(Uint32 i) noexcept;

	// Converts given uint64 value `i` into a 64-bit floating
	// point value in the range of [0.0, 1.0)
	template <class Uint64, std::enable_if_t<std::is_same_v<Uint64, std::uint64_t>>* = nullptr>
	[[nodiscard]]
	inline constexpr double DoubleFromBits(Uint64 i) noexcept;

	// SplitMix64
	// Output: 64 bits
	// Period: 2^64
	// Footprint: 8 bytes
	// Original implementation: http://prng.di.unimi.it/splitmix64.c
	class SplitMix64
	{
	public:

		using state_type	= std::uint64_t;	
		using result_type	= std::uint64_t;
		
		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr SplitMix64(state_type state = DefaultSeed) noexcept;

		constexpr result_type operator()() noexcept;

		template <std::size_t N>
		[[nodiscard]]
		constexpr std::array<std::uint64_t, N> generateSeedSequence() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const SplitMix64& lhs, const SplitMix64& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const SplitMix64& lhs, const SplitMix64& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}
	
	private:

		state_type m_state;
	};

	// xoshiro256+
	// Output: 64 bits
	// Period: 2^256 - 1
	// Footprint: 32 bytes
	// Original implementation: http://prng.di.unimi.it/xoshiro256plus.c
	// Version: 1.0
	class Xoshiro256Plus
	{
	public:

		using state_type	= std::array<std::uint64_t, 4>;
		using result_type	= std::uint64_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro256Plus(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro256Plus(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^128 calls to operator(); it can be used to generate 2^128
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^192 calls to next(); it can be used to generate 2^64 starting points,
		// from each of which jump() will generate 2^64 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoshiro256Plus& lhs, const Xoshiro256Plus& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoshiro256Plus& lhs, const Xoshiro256Plus& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoshiro256++
	// Output: 64 bits
	// Period: 2^256 - 1
	// Footprint: 32 bytes
	// Original implementation: http://prng.di.unimi.it/xoshiro256plusplus.c
	// Version: 1.0
	class Xoshiro256PlusPlus
	{
	public:

		using state_type	= std::array<std::uint64_t, 4>;
		using result_type	= std::uint64_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro256PlusPlus(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro256PlusPlus(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^128 calls to next(); it can be used to generate 2^128
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^192 calls to next(); it can be used to generate 2^64 starting points,
		// from each of which jump() will generate 2^64 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoshiro256PlusPlus& lhs, const Xoshiro256PlusPlus& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoshiro256PlusPlus& lhs, const Xoshiro256PlusPlus& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoshiro256**
	// Output: 64 bits
	// Period: 2^256 - 1
	// Footprint: 32 bytes
	// Original implementation: http://prng.di.unimi.it/xoshiro256starstar.c
	// Version: 1.0
	class Xoshiro256StarStar
	{
	public:

		using state_type	= std::array<std::uint64_t, 4>;
		using result_type	= std::uint64_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro256StarStar(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro256StarStar(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^128 calls to next(); it can be used to generate 2^128
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^192 calls to next(); it can be used to generate 2^64 starting points,
		// from each of which jump() will generate 2^64 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoshiro256StarStar& lhs, const Xoshiro256StarStar& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoshiro256StarStar& lhs, const Xoshiro256StarStar& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoroshiro128+
	// Output: 64 bits
	// Period: 2^128 - 1
	// Footprint: 16 bytes
	// Original implementation: http://prng.di.unimi.it/xoroshiro128plus.c
	// Version: 1.0
	class Xoroshiro128Plus
	{
	public:

		using state_type	= std::array<std::uint64_t, 2>;
		using result_type	= std::uint64_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoroshiro128Plus(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoroshiro128Plus(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^64 calls to next(); it can be used to generate 2^64
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^96 calls to next(); it can be used to generate 2^32 starting points,
		// from each of which jump() will generate 2^32 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoroshiro128Plus& lhs, const Xoroshiro128Plus& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoroshiro128Plus& lhs, const Xoroshiro128Plus& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoroshiro128++
	// Output: 64 bits
	// Period: 2^128 - 1
	// Footprint: 16 bytes
	// Original implementation: http://prng.di.unimi.it/xoroshiro128plusplus.c
	// Version: 1.0
	class Xoroshiro128PlusPlus
	{
	public:

		using state_type	= std::array<std::uint64_t, 2>;
		using result_type	= std::uint64_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoroshiro128PlusPlus(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoroshiro128PlusPlus(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^64 calls to next(); it can be used to generate 2^64
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^96 calls to next(); it can be used to generate 2^32 starting points,
		// from each of which jump() will generate 2^32 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoroshiro128PlusPlus& lhs, const Xoroshiro128PlusPlus& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoroshiro128PlusPlus& lhs, const Xoroshiro128PlusPlus& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoroshiro128**
	// Output: 64 bits
	// Period: 2^128 - 1
	// Footprint: 16 bytes
	// Original implementation: http://prng.di.unimi.it/xoroshiro128starstar.c
	// Version: 1.0
	class Xoroshiro128StarStar
	{
	public:

		using state_type	= std::array<std::uint64_t, 2>;
		using result_type	= std::uint64_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoroshiro128StarStar(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoroshiro128StarStar(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^64 calls to next(); it can be used to generate 2^64
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^96 calls to next(); it can be used to generate 2^32 starting points,
		// from each of which jump() will generate 2^32 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoroshiro128StarStar& lhs, const Xoroshiro128StarStar& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoroshiro128StarStar& lhs, const Xoroshiro128StarStar& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoshiro128+
	// Output: 32 bits
	// Period: 2^128 - 1
	// Footprint: 16 bytes
	// Original implementation: http://prng.di.unimi.it/xoshiro128plus.c
	// Version: 1.0
	class Xoshiro128Plus
	{
	public:

		using state_type	= std::array<std::uint32_t, 4>;
		using result_type	= std::uint32_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro128Plus(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro128Plus(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^64 calls to next(); it can be used to generate 2^64
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^96 calls to next(); it can be used to generate 2^32 starting points,
		// from each of which jump() will generate 2^32 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoshiro128Plus& lhs, const Xoshiro128Plus& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoshiro128Plus& lhs, const Xoshiro128Plus& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoshiro128++
	// Output: 32 bits
	// Period: 2^128 - 1
	// Footprint: 16 bytes
	// Original implementation: http://prng.di.unimi.it/xoshiro128plusplus.c
	// Version: 1.0
	class Xoshiro128PlusPlus
	{
	public:

		using state_type	= std::array<std::uint32_t, 4>;
		using result_type	= std::uint32_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro128PlusPlus(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro128PlusPlus(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^64 calls to next(); it can be used to generate 2^64
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^96 calls to next(); it can be used to generate 2^32 starting points,
		// from each of which jump() will generate 2^32 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoshiro128PlusPlus& lhs, const Xoshiro128PlusPlus& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoshiro128PlusPlus& lhs, const Xoshiro128PlusPlus& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};

	// xoshiro128**
	// Output: 32 bits
	// Period: 2^128 - 1
	// Footprint: 16 bytes
	// Original implementation: http://prng.di.unimi.it/xoshiro128starstar.c
	// Version: 1.1
	class Xoshiro128StarStar
	{
	public:

		using state_type	= std::array<std::uint32_t, 4>;
		using result_type	= std::uint32_t;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro128StarStar(std::uint64_t seed = DefaultSeed) noexcept;

		XOSHIROCPP_NODISCARD_CXX20
		explicit constexpr Xoshiro128StarStar(state_type state) noexcept;

		constexpr result_type operator()() noexcept;

		// This is the jump function for the generator. It is equivalent
		// to 2^64 calls to next(); it can be used to generate 2^64
		// non-overlapping subsequences for parallel computations.
		constexpr void jump() noexcept;

		// This is the long-jump function for the generator. It is equivalent to
		// 2^96 calls to next(); it can be used to generate 2^32 starting points,
		// from each of which jump() will generate 2^32 non-overlapping
		// subsequences for parallel distributed computations.
		constexpr void longJump() noexcept;

		[[nodiscard]]
		static constexpr result_type min() noexcept;

		[[nodiscard]]
		static constexpr result_type max() noexcept;

		[[nodiscard]]
		constexpr state_type serialize() const noexcept;

		constexpr void deserialize(state_type state) noexcept;

		[[nodiscard]]
		friend bool operator ==(const Xoshiro128StarStar& lhs, const Xoshiro128StarStar& rhs) noexcept
		{
			return (lhs.m_state == rhs.m_state);
		}

		[[nodiscard]]
		friend bool operator !=(const Xoshiro128StarStar& lhs, const Xoshiro128StarStar& rhs) noexcept
		{
			return (lhs.m_state != rhs.m_state);
		}

	private:

		state_type m_state;
	};
}

////////////////////////////////////////////////////////////////

namespace XoshiroCpp
{
	template <class Uint32, std::enable_if_t<std::is_same_v<Uint32, std::uint32_t>>*>
	inline constexpr float FloatFromBits(const Uint32 i) noexcept
	{
		return (i >> 8) * 0x1.0p-24f;
	}

	template <class Uint64, std::enable_if_t<std::is_same_v<Uint64, std::uint64_t>>*>
	inline constexpr double DoubleFromBits(const Uint64 i) noexcept
	{
		return (i >> 11) * 0x1.0p-53;
	}

	namespace detail
	{
		[[nodiscard]]
		static constexpr std::uint64_t RotL(const std::uint64_t x, const int s) noexcept
		{
			return (x << s) | (x >> (64 - s));
		}

		[[nodiscard]]
		static constexpr std::uint32_t RotL(const std::uint32_t x, const int s) noexcept
		{
			return (x << s) | (x >> (32 - s));
		}
	}

	////////////////////////////////////////////////////////////////
	//
	//	SplitMix64
	//
	inline constexpr SplitMix64::SplitMix64(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr SplitMix64::result_type SplitMix64::operator()() noexcept
	{
		std::uint64_t z = (m_state += 0x9e3779b97f4a7c15);
		z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
		z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
		return z ^ (z >> 31);
	}

	template <std::size_t N>
	inline constexpr std::array<std::uint64_t, N> SplitMix64::generateSeedSequence() noexcept
	{
		std::array<std::uint64_t, N> seeds = {};

		for (auto& seed : seeds)
		{
			seed = operator()();
		}

		return seeds;
	}

	inline constexpr SplitMix64::result_type SplitMix64::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr SplitMix64::result_type SplitMix64::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr SplitMix64::state_type SplitMix64::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void SplitMix64::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoshiro256+
	//
	inline constexpr Xoshiro256Plus::Xoshiro256Plus(const std::uint64_t seed) noexcept
		: m_state(SplitMix64{ seed }.generateSeedSequence<4>()) {}

	inline constexpr Xoshiro256Plus::Xoshiro256Plus(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoshiro256Plus::result_type Xoshiro256Plus::operator()() noexcept
	{
		const std::uint64_t result = m_state[0] + m_state[3];
		const std::uint64_t t = m_state[1] << 17;
		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];
		m_state[2] ^= t;
		m_state[3] = detail::RotL(m_state[3], 45);
		return result;
	}

	inline constexpr void Xoshiro256Plus::jump() noexcept
	{
		constexpr std::uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		std::uint64_t s2 = 0;
		std::uint64_t s3 = 0;

		for (std::uint64_t jump : JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr void Xoshiro256Plus::longJump() noexcept
	{
		constexpr std::uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		std::uint64_t s2 = 0;
		std::uint64_t s3 = 0;

		for (std::uint64_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr Xoshiro256Plus::result_type Xoshiro256Plus::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoshiro256Plus::result_type Xoshiro256Plus::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoshiro256Plus::state_type Xoshiro256Plus::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoshiro256Plus::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoshiro256++
	//
	inline constexpr Xoshiro256PlusPlus::Xoshiro256PlusPlus(const std::uint64_t seed) noexcept
		: m_state(SplitMix64{ seed }.generateSeedSequence<4>()) {}

	inline constexpr Xoshiro256PlusPlus::Xoshiro256PlusPlus(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoshiro256PlusPlus::result_type Xoshiro256PlusPlus::operator()() noexcept
	{
		const std::uint64_t result = detail::RotL(m_state[0] + m_state[3], 23) + m_state[0];
		const std::uint64_t t = m_state[1] << 17;
		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];
		m_state[2] ^= t;
		m_state[3] = detail::RotL(m_state[3], 45);
		return result;
	}

	inline constexpr void Xoshiro256PlusPlus::jump() noexcept
	{
		constexpr std::uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		std::uint64_t s2 = 0;
		std::uint64_t s3 = 0;

		for (std::uint64_t jump : JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr void Xoshiro256PlusPlus::longJump() noexcept
	{
		constexpr std::uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		std::uint64_t s2 = 0;
		std::uint64_t s3 = 0;

		for (std::uint64_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr Xoshiro256PlusPlus::result_type Xoshiro256PlusPlus::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoshiro256PlusPlus::result_type Xoshiro256PlusPlus::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoshiro256PlusPlus::state_type Xoshiro256PlusPlus::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoshiro256PlusPlus::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoshiro256**
	//
	inline constexpr Xoshiro256StarStar::Xoshiro256StarStar(const std::uint64_t seed) noexcept
		: m_state(SplitMix64{ seed }.generateSeedSequence<4>()) {}

	inline constexpr Xoshiro256StarStar::Xoshiro256StarStar(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoshiro256StarStar::result_type Xoshiro256StarStar::operator()() noexcept
	{
		const std::uint64_t result = detail::RotL(m_state[1] * 5, 7) * 9;
		const std::uint64_t t = m_state[1] << 17;
		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];
		m_state[2] ^= t;
		m_state[3] = detail::RotL(m_state[3], 45);
		return result;
	}

	inline constexpr void Xoshiro256StarStar::jump() noexcept
	{
		constexpr std::uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		std::uint64_t s2 = 0;
		std::uint64_t s3 = 0;

		for (std::uint64_t jump : JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr void Xoshiro256StarStar::longJump() noexcept
	{
		constexpr std::uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;
		std::uint64_t s2 = 0;
		std::uint64_t s3 = 0;

		for (std::uint64_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr Xoshiro256StarStar::result_type Xoshiro256StarStar::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoshiro256StarStar::result_type Xoshiro256StarStar::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoshiro256StarStar::state_type Xoshiro256StarStar::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoshiro256StarStar::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoroshiro128+
	//
	inline constexpr Xoroshiro128Plus::Xoroshiro128Plus(const std::uint64_t seed) noexcept
		: m_state(SplitMix64{ seed }.generateSeedSequence<2>()) {}

	inline constexpr Xoroshiro128Plus::Xoroshiro128Plus(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoroshiro128Plus::result_type Xoroshiro128Plus::operator()() noexcept
	{
		const std::uint64_t s0 = m_state[0];
		std::uint64_t s1 = m_state[1];
		const std::uint64_t result = s0 + s1;
		s1 ^= s0;
		m_state[0] = detail::RotL(s0, 24) ^ s1 ^ (s1 << 16);
		m_state[1] = detail::RotL(s1, 37);
		return result;
	}

	inline constexpr void Xoroshiro128Plus::jump() noexcept
	{
		constexpr std::uint64_t JUMP[] = { 0xdf900294d8f554a5, 0x170865df4b3201fc };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;

		for (std::uint64_t jump : JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
	}

	inline constexpr void Xoroshiro128Plus::longJump() noexcept
	{
		constexpr std::uint64_t LONG_JUMP[] = { 0xd2a98b26625eee7b, 0xdddf9b1090aa7ac1 };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;

		for (std::uint64_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
	}

	inline constexpr Xoroshiro128Plus::result_type Xoroshiro128Plus::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoroshiro128Plus::result_type Xoroshiro128Plus::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoroshiro128Plus::state_type Xoroshiro128Plus::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoroshiro128Plus::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoroshiro128++
	//
	inline constexpr Xoroshiro128PlusPlus::Xoroshiro128PlusPlus(const std::uint64_t seed) noexcept
		: m_state(SplitMix64{ seed }.generateSeedSequence<2>()) {}

	inline constexpr Xoroshiro128PlusPlus::Xoroshiro128PlusPlus(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoroshiro128PlusPlus::result_type Xoroshiro128PlusPlus::operator()() noexcept
	{
		const std::uint64_t s0 = m_state[0];
		std::uint64_t s1 = m_state[1];
		const std::uint64_t result = detail::RotL(s0 + s1, 17) + s0;
		s1 ^= s0;
		m_state[0] = detail::RotL(s0, 49) ^ s1 ^ (s1 << 21);
		m_state[1] = detail::RotL(s1, 28);
		return result;
	}

	inline constexpr void Xoroshiro128PlusPlus::jump() noexcept
	{
		constexpr std::uint64_t JUMP[] = { 0x2bd7a6a6e99c2ddc, 0x0992ccaf6a6fca05 };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;

		for (std::uint64_t jump : JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
	}

	inline constexpr void Xoroshiro128PlusPlus::longJump() noexcept
	{
		constexpr std::uint64_t LONG_JUMP[] = { 0x360fd5f2cf8d5d99, 0x9c6e6877736c46e3 };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;

		for (std::uint64_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
	}

	inline constexpr Xoroshiro128PlusPlus::result_type Xoroshiro128PlusPlus::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoroshiro128PlusPlus::result_type Xoroshiro128PlusPlus::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoroshiro128PlusPlus::state_type Xoroshiro128PlusPlus::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoroshiro128PlusPlus::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoroshiro128**
	//
	inline constexpr Xoroshiro128StarStar::Xoroshiro128StarStar(const std::uint64_t seed) noexcept
		: m_state(SplitMix64{ seed }.generateSeedSequence<2>()) {}

	inline constexpr Xoroshiro128StarStar::Xoroshiro128StarStar(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoroshiro128StarStar::result_type Xoroshiro128StarStar::operator()() noexcept
	{
		const std::uint64_t s0 = m_state[0];
		std::uint64_t s1 = m_state[1];
		const std::uint64_t result = detail::RotL(s0 * 5, 7) * 9;
		s1 ^= s0;
		m_state[0] = detail::RotL(s0, 24) ^ s1 ^ (s1 << 16);
		m_state[1] = detail::RotL(s1, 37);
		return result;
	}

	inline constexpr void Xoroshiro128StarStar::jump() noexcept
	{
		constexpr std::uint64_t JUMP[] = { 0xdf900294d8f554a5, 0x170865df4b3201fc };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;

		for (std::uint64_t jump : JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
	}

	inline constexpr void Xoroshiro128StarStar::longJump() noexcept
	{
		constexpr std::uint64_t LONG_JUMP[] = { 0xd2a98b26625eee7b, 0xdddf9b1090aa7ac1 };

		std::uint64_t s0 = 0;
		std::uint64_t s1 = 0;

		for (std::uint64_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 64; ++b)
			{
				if (jump & UINT64_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
	}

	inline constexpr Xoroshiro128StarStar::result_type Xoroshiro128StarStar::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoroshiro128StarStar::result_type Xoroshiro128StarStar::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoroshiro128StarStar::state_type Xoroshiro128StarStar::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoroshiro128StarStar::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoshiro128+
	//
	inline constexpr Xoshiro128Plus::Xoshiro128Plus(const std::uint64_t seed) noexcept
		: m_state()
	{
		SplitMix64 splitmix{ seed };

		for (auto& state : m_state)
		{
			state = static_cast<std::uint32_t>(splitmix());
		}
	}

	inline constexpr Xoshiro128Plus::Xoshiro128Plus(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoshiro128Plus::result_type Xoshiro128Plus::operator()() noexcept
	{
		const std::uint32_t result = m_state[0] + m_state[3];
		const std::uint32_t t = m_state[1] << 9;
		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];
		m_state[2] ^= t;
		m_state[3] = detail::RotL(m_state[3], 11);
		return result;
	}

	inline constexpr void Xoshiro128Plus::jump() noexcept
	{
		constexpr std::uint32_t JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

		std::uint32_t s0 = 0;
		std::uint32_t s1 = 0;
		std::uint32_t s2 = 0;
		std::uint32_t s3 = 0;

		for (std::uint32_t jump : JUMP)
		{
			for (int b = 0; b < 32; ++b)
			{
				if (jump & UINT32_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr void Xoshiro128Plus::longJump() noexcept
	{
		constexpr std::uint32_t LONG_JUMP[] = { 0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662 };

		std::uint32_t s0 = 0;
		std::uint32_t s1 = 0;
		std::uint32_t s2 = 0;
		std::uint32_t s3 = 0;

		for (std::uint32_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 32; ++b)
			{
				if (jump & UINT32_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr Xoshiro128Plus::result_type Xoshiro128Plus::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoshiro128Plus::result_type Xoshiro128Plus::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoshiro128Plus::state_type Xoshiro128Plus::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoshiro128Plus::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoshiro128++
	//
	inline constexpr Xoshiro128PlusPlus::Xoshiro128PlusPlus(const std::uint64_t seed) noexcept
		: m_state()
	{
		SplitMix64 splitmix{ seed };

		for (auto& state : m_state)
		{
			state = static_cast<std::uint32_t>(splitmix());
		}
	}

	inline constexpr Xoshiro128PlusPlus::Xoshiro128PlusPlus(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoshiro128PlusPlus::result_type Xoshiro128PlusPlus::operator()() noexcept
	{
		const std::uint32_t result = detail::RotL(m_state[0] + m_state[3], 7) + m_state[0];
		const std::uint32_t t = m_state[1] << 9;
		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];
		m_state[2] ^= t;
		m_state[3] = detail::RotL(m_state[3], 11);
		return result;
	}

	inline constexpr void Xoshiro128PlusPlus::jump() noexcept
	{
		constexpr std::uint32_t JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

		std::uint32_t s0 = 0;
		std::uint32_t s1 = 0;
		std::uint32_t s2 = 0;
		std::uint32_t s3 = 0;

		for (std::uint32_t jump : JUMP)
		{
			for (int b = 0; b < 32; ++b)
			{
				if (jump & UINT32_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr void Xoshiro128PlusPlus::longJump() noexcept
	{
		constexpr std::uint32_t LONG_JUMP[] = { 0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662 };

		std::uint32_t s0 = 0;
		std::uint32_t s1 = 0;
		std::uint32_t s2 = 0;
		std::uint32_t s3 = 0;

		for (std::uint32_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 32; ++b)
			{
				if (jump & UINT32_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr Xoshiro128PlusPlus::result_type Xoshiro128PlusPlus::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoshiro128PlusPlus::result_type Xoshiro128PlusPlus::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoshiro128PlusPlus::state_type Xoshiro128PlusPlus::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoshiro128PlusPlus::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}

	////////////////////////////////////////////////////////////////
	//
	//	xoshiro128**
	//
	inline constexpr Xoshiro128StarStar::Xoshiro128StarStar(const std::uint64_t seed) noexcept
		: m_state()
	{
		SplitMix64 splitmix{ seed };

		for (auto& state : m_state)
		{
			state = static_cast<std::uint32_t>(splitmix());
		}
	}

	inline constexpr Xoshiro128StarStar::Xoshiro128StarStar(const state_type state) noexcept
		: m_state(state) {}

	inline constexpr Xoshiro128StarStar::result_type Xoshiro128StarStar::operator()() noexcept
	{
		const std::uint32_t result = detail::RotL(m_state[1] * 5, 7) * 9;
		const std::uint32_t t = m_state[1] << 9;
		m_state[2] ^= m_state[0];
		m_state[3] ^= m_state[1];
		m_state[1] ^= m_state[2];
		m_state[0] ^= m_state[3];
		m_state[2] ^= t;
		m_state[3] = detail::RotL(m_state[3], 11);
		return result;
	}

	inline constexpr void Xoshiro128StarStar::jump() noexcept
	{
		constexpr std::uint32_t JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

		std::uint32_t s0 = 0;
		std::uint32_t s1 = 0;
		std::uint32_t s2 = 0;
		std::uint32_t s3 = 0;

		for (std::uint32_t jump : JUMP)
		{
			for (int b = 0; b < 32; ++b)
			{
				if (jump & UINT32_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr void Xoshiro128StarStar::longJump() noexcept
	{
		constexpr std::uint32_t LONG_JUMP[] = { 0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662 };

		std::uint32_t s0 = 0;
		std::uint32_t s1 = 0;
		std::uint32_t s2 = 0;
		std::uint32_t s3 = 0;

		for (std::uint32_t jump : LONG_JUMP)
		{
			for (int b = 0; b < 32; ++b)
			{
				if (jump & UINT32_C(1) << b)
				{
					s0 ^= m_state[0];
					s1 ^= m_state[1];
					s2 ^= m_state[2];
					s3 ^= m_state[3];
				}
				operator()();
			}
		}

		m_state[0] = s0;
		m_state[1] = s1;
		m_state[2] = s2;
		m_state[3] = s3;
	}

	inline constexpr Xoshiro128StarStar::result_type Xoshiro128StarStar::min() noexcept
	{
		return std::numeric_limits<result_type>::lowest();
	}

	inline constexpr Xoshiro128StarStar::result_type Xoshiro128StarStar::max() noexcept
	{
		return std::numeric_limits<result_type>::max();
	}

	inline constexpr Xoshiro128StarStar::state_type Xoshiro128StarStar::serialize() const noexcept
	{
		return m_state;
	}

	inline constexpr void Xoshiro128StarStar::deserialize(const state_type state) noexcept
	{
		m_state = state;
	}
}
