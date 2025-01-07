#include <glaze/glaze.hpp>

#include <print>
#include <vector>
#include <cstdio>
#include <string>
#include <cstdint>
#include <utility>
#include <string_view>

/*
    This program generates the default strings (english) from a csv file.
*/

enum ColIdx : std::uint32_t
{
    Key,
    Default,
    Translation
};

struct LanguageDoc
{
    std::vector<std::string> Key;
    std::vector<std::string> Default;
    std::vector<std::string> Translation;

    auto GetRowCount(void) const { return Key.size(); }
};

template <>
struct glz::meta<LanguageDoc>
{
    using T = LanguageDoc;
    static constexpr auto value = glz::object(
        "Key (do not touch)", &T::Key,
        "Default", &T::Default,
        "Translation", &T::Translation
    );
};

constexpr std::string_view HeaderHead = R"(#pragma once
#include <cstdint>
#include <array>
#include <unordered_map>
#include <string>

enum class Tr : uint32_t
{
    )";

constexpr std::string_view HeaderFooter = R"(
};

struct OFS_DefaultStrings
{
    static std::array<const char*, static_cast<uint32_t>(Tr::MAX_STRING_COUNT)> Default;
    static std::unordered_map<std::string, Tr> KeyMapping;
};
)";


constexpr std::string_view SrcArrayHead = R"(#include "OFS_StringsGenerated.h"
std::array<const char*, static_cast<uint32_t>(Tr::MAX_STRING_COUNT)> OFS_DefaultStrings::Default =
{
    )";

constexpr std::string_view SrcArrayFooter = R"(
};
)";

constexpr std::string_view SrcMappingHead = R"(
std::unordered_map<std::string, Tr> OFS_DefaultStrings::KeyMapping =
{
)";
constexpr std::string_view SrcMappingFooter = R"(
};
)";

static void write_src_file(FILE* src, LanguageDoc const& doc) noexcept
{
    // write the array
    std::print(src, "{} R\"({})\",\n\t", SrcArrayHead, doc.Default[0]);

    for (size_t n = 1; n < doc.GetRowCount(); ++n)
    {
        std::print(src, "R\"({})\",\n\t", doc.Default[n]);
    }

    // write the key to enum hashmap
    std::print(src, "{0}{1}\t{{\"{2}\", Tr::{2}}},\n", SrcArrayFooter, SrcMappingHead, doc.Key[0]);

    for (size_t n = 1; n < doc.GetRowCount(); ++n)
    {
        std::print(src, "\t{{\"{0}\", Tr::{0}}},\n", doc.Key[n]);
    }
    std::print(src, "{}", SrcMappingFooter);
    std::fclose(src);
}

static void write_header_enum(FILE* header, LanguageDoc const& doc) noexcept
{
    std::print(header, "{}{}", HeaderHead, doc.Key[0]);
    for(size_t n=1; n < doc.GetRowCount(); ++n)
    {
        std::print(header, ",\n\t{}", doc.Key[n]);
    }

    std::print(header, ",\n\tMAX_STRING_COUNT{}", HeaderFooter);
    std::fclose(header);
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::println("Usage:\n{} [csv file] [output dir]", argv[0]);
        return -1;
    }

    auto csvFile = argv[1];
    auto header = fopen((std::string(argv[2]) + "OFS_StringsGenerated.h").c_str(), "wb");
    auto src = fopen((std::string(argv[2]) + "OFS_StringsGenerated.cpp").c_str(), "wb");
    if(header == nullptr || src == nullptr)
    {
        std::println("Failed to create source files.");
        return -1;
    }

    try
    {
        LanguageDoc doc{};
        if (auto err = glz::read_file_csv<glz::colwise>(doc, csvFile, std::string{}); err)
        {
            std::print("Error reading csv file: {}, code {}\n", err.custom_error_message, std::to_underlying(err.ec));
            return -1;
        }
        
        std::print("lines read {}", doc.GetRowCount());
        write_header_enum(header, doc);
        write_src_file(src, doc);
    }
    catch(const std::exception& e)
    {
        std::println("Error: {}", e.what());
    }

    return 0;
}