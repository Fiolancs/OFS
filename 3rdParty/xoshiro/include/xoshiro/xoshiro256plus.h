#pragma once

#include <cstdint>

namespace xoshiro
{
	struct xoshiro256plus
	{
		xoshiro256plus(std::uint64_t seed) noexcept;
		xoshiro256plus(std::uint64_t s0, std::uint64_t s1, std::uint64_t s2, std::uint64_t s3) noexcept;
		std::uint64_t operator () (void) noexcept;

	private:
		std::uint64_t s[4];
	};
}
