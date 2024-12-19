#pragma once
#include "OFS_VectorSet.h"
//#include "OFS_BinarySerialization.h"

#include <bit>
#include <limits>
#include <cstdint>

namespace OFS
{
	namespace v2
	{
		struct FunscriptAction
		{
			std::uint32_t at;
			std::uint8_t  pos;
			std::uint8_t  tag;
			std::uint8_t  flags; // unused
		};

		static_assert(sizeof(FunscriptAction) <= 8);
	}

	inline namespace v1
	{
		struct FunscriptAction
		{
		public:
			// timestamp as floating point seconds
			// instead of integer milliseconds
			float atS;
			std::int8_t pos;
			std::uint8_t flags; // unused
			std::uint8_t tag;

			//template <typename S>
			//void serialize(S& s)
			//{
			//	s.ext(*this, bitsery::ext::Growable{},
			//		[](S& s, FunscriptAction& o) {
			//			s.value4b(o.atS);
			//			s.value2b(o.pos);
			//			s.value1b(o.flags);
			//			s.value1b(o.tag);
			//		});
			//}

			FunscriptAction(void) noexcept
				: atS(std::numeric_limits<decltype(atS)>::min())
				, pos(std::numeric_limits<decltype(pos)>::min())
				, flags(0), tag(0)
			{
			}

			FunscriptAction(float at, std::int8_t pos) noexcept
				: FunscriptAction(at, pos, 0)
			{
			}

			FunscriptAction(float at, std::int8_t pos, std::uint8_t tag) noexcept
				: atS(at), pos(pos), flags(0), tag(tag)
			{
			}

			inline bool operator <  (FunscriptAction b) const noexcept { return this->atS < b.atS; }
			inline bool operator == (FunscriptAction b) const noexcept { return this->atS == b.atS && this->pos == b.pos; }
			inline bool operator != (FunscriptAction b) const noexcept { return !(*this == b); }
		};

		struct FunscriptActionHashfunction
		{
			inline std::uint64_t operator()(FunscriptAction s) const noexcept
			{
				static_assert(sizeof(FunscriptAction) == sizeof(std::uint64_t));
				return std::bit_cast<std::uint64_t>(s);
			}
		};
	}
}

using OFS::FunscriptAction;
using FunscriptArray = vector_set<FunscriptAction>;
