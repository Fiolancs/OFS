#pragma once
#include "state/OFS_StateHandle.h"
#include "io/OFS_BinarySerialization.h"

#include "io/OFS_SerializeHelper.h"

//#include <stb/sdefl.h>
//#include <stb/sinfl.h>

#include <limits>
#include <string>
#include <vector>
#include <cstdint>

// QQQ
struct WaveformState
{
    static constexpr auto StateName = "WaveformState";
    std::string Filename;
    std::vector<std::uint8_t> BinSamples;
    size_t UncompressedSize = 0;

    std::vector<float> GetSamples() noexcept
    {
        // QQQ
        //if(UncompressedSize == 0) 
        //    return {};
        //std::vector<uint8_t> decompressed;
        //decompressed.resize(UncompressedSize);
        //
        //auto size = sinflate(decompressed.data(), decompressed.size(), BinSamples.data(), BinSamples.size());
        //if(size == UncompressedSize)
        //{
        //    std::vector<uint16_t> u16Samples;
        //    OFS_Binary::Deserialize(decompressed, u16Samples);
        //    std::vector<float> samples;
        //    samples.reserve(u16Samples.size());
        //    for(auto sample : u16Samples)
        //        samples.emplace_back(sample / (float) std::numeric_limits<uint16_t>::max());
        //    return samples;
        //}
        return {};
    }

    void SetSamples(const std::vector<float>& samples)
    {
        // QQQ
        //std::vector<uint16_t> u16Samples;
        //u16Samples.reserve(samples.size());
        //for(auto sample : samples)
        //    u16Samples.emplace_back((uint16_t)(sample * (float)std::numeric_limits<uint16_t>::max()));
        //
        //BinSamples.clear();
        //auto size = OFS_Binary::Serialize(BinSamples, u16Samples);
        //BinSamples.resize(size);
        //
        //UncompressedSize = size;
        //
        //std::vector<uint8_t> compressedBin;
        //compressedBin.resize(size);
        //
        //sdefl ctx = {0};
        //auto compressedSize = sdeflate(&ctx, compressedBin.data(), BinSamples.data(), BinSamples.size(), 8);
        //compressedBin.resize(compressedSize);
        //BinSamples = std::move(compressedBin);
    }

    inline static WaveformState& StaticStateSlow() noexcept
    {
        // This shouldn't be done in hot paths but shouldn't be a problem otherwise.
        auto handle = OFS::ProjectState<WaveformState>::registerState(StateName, StateName);
        return OFS::ProjectState<WaveformState>(handle).get();
    }
};

//REFL_TYPE(WaveformState)
//    REFL_FIELD(Filename)
//    REFL_FIELD(BinSamples)
//    REFL_FIELD(UncompressedSize)
//REFL_END
