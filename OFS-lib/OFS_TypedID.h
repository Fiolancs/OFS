#pragma once

#include <cstdint>
#include <compare>
#include <functional>  // std::hash

namespace OFS
{
	template <typename T>
	struct TypedID
	{
		std::uint32_t value;

		constexpr std::strong_ordering operator <=> (TypedID const&) const noexcept = default;
		constexpr explicit operator std::uint32_t(void) const noexcept { return value; }
	};
}

template <typename T>
struct std::hash <OFS::TypedID<T>>
{
	auto operator() (OFS::TypedID<T> const& value) const noexcept
	{
		return hash<decltype(OFS::TypedID<T>::value)>{}(value.value);
	}
};
