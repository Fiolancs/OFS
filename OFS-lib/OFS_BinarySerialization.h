#pragma once
#include "OFS_VectorSet.h"
#include "UI/OFS_Profiling.h"

#include <zpp_bits.h>

#include <vector>
#include <cstdint>

// QQQ

using ByteBuffer = std::vector<std::uint8_t>;
//using OutputAdapter = bitsery::OutputBufferAdapter<ByteBuffer>;
//using InputAdapter = bitsery::InputBufferAdapter<ByteBuffer>;
//
//using TContext = std::tuple<bitsery::ext::PointerLinkingContext>;
//
//using ContextSerializer = bitsery::Serializer<OutputAdapter, TContext>;
//using ContextDeserializer = bitsery::Deserializer<InputAdapter, TContext>;

struct OFS_Binary {

    template<typename T>
    static size_t Serialize(ByteBuffer& buffer, T& obj) noexcept
    {
        OFS_PROFILE(__FUNCTION__);
        return 0;
        //zpp::bits::out out{ buffer };
        //out(obj);
        //return out.position();

        //TContext ctx{};
        //
        //ContextSerializer ser{ ctx, buffer };
        //ser.object(obj);
        //ser.adapter().flush();
        //auto writtenSize = ser.adapter().writtenBytesCount();
        //
        //return writtenSize;
    }

    template<typename T>
    static auto Deserialize(ByteBuffer& buffer, T& obj) noexcept
    {
        OFS_PROFILE(__FUNCTION__);
        //zpp::bits::in in{ buffer };
        //return in(obj);
        return false;
        //TContext ctx{};
        //ContextDeserializer des{ ctx, buffer.begin(), buffer.size() };
        //des.object(obj);
        //
        //auto error = des.adapter().error();
        //auto valid = std::get<0>(ctx).isValid();
        //std::get<0>(ctx).clearSharedState();
        //
        //return error;
    }
};
//
//
//namespace bitsery {
//    template<typename S>
//    void serialize(S& s, ImVec2& o)
//    {
//        s.value4b(o.x);
//        s.value4b(o.y);
//    }
//
//    template<typename S>
//    void serialize(S& s, ImVec4& o)
//    {
//        s.value4b(o.x);
//        s.value4b(o.y);
//        s.value4b(o.z);
//        s.value4b(o.w);
//    }
//
//    template<typename S>
//    void serialize(S& s, ImColor& o)
//    {
//        s.object(o.Value);
//    }
//
//    template<typename S>
//    void serialize(S& s, std::vector<float>& o)
//    {
//        s.container(o, std::numeric_limits<uint32_t>::max(),
//            [](S& s, float& v) {
//                s.value4b(v);
//            });
//    }
//
//    template<typename S>
//    void serialize(S& s, std::vector<uint16_t>& o)
//    {
//        s.container(o, std::numeric_limits<uint32_t>::max(),
//            [](S& s, uint16_t& v) {
//                s.value2b(v);
//            });
//    }
//}