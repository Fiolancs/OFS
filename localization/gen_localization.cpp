#include <glaze/glaze.hpp>

#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdint>
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
    fwrite(SrcArrayHead.data(), sizeof(char), SrcArrayHead.size(), src);
    fwrite("R\"(", sizeof(char), sizeof("R\"(") - 1, src);
    fwrite(doc.Default[0].data(), sizeof(doc.Default[0][0]), doc.Default[0].size(), src);
    fwrite(")\",\n\t", sizeof(char), sizeof(")\",\n\t") - 1, src);

    for (size_t n = 1; n < doc.GetRowCount(); ++n)
    {
        fwrite("R\"(", sizeof(char), sizeof("R\"(") - 1, src);
        fwrite(doc.Default[n].data(), sizeof(char), doc.Default[n].size(), src);
        fwrite(")\",\n\t", 1, sizeof(")\",\n\t") - 1, src);
    }
    fwrite(SrcArrayFooter.data(), 1, SrcArrayFooter.size(), src);

    // write the key to enum hashmap
    fwrite(SrcMappingHead.data(), 1, SrcMappingHead.size(), src);
    fwrite("\t{\"", 1, sizeof("\t{\"")-1, src);
    fwrite(doc.Key[0].data(), sizeof(doc.Key[0][0]), doc.Key[0].size(), src);
    fwrite("\", Tr::", sizeof(char), sizeof("\", Tr::") - 1, src);
    fwrite(doc.Key[0].data(), sizeof(doc.Key[0][0]), doc.Key[0].size(), src);
    fwrite("},\n", sizeof(char), sizeof("},\n") - 1, src);
    for (size_t n = 1; n < doc.GetRowCount(); ++n)
    {
        fwrite("\t{\"", sizeof(char), sizeof("\t{\"")-1, src);
        fwrite(doc.Key[0].data(), sizeof(doc.Key[0][0]), doc.Key[0].size(), src);
        fwrite("\", Tr::", 1, sizeof("\", Tr::")-1, src);
        fwrite(doc.Key[0].data(), sizeof(doc.Key[0][0]), doc.Key[0].size(), src);
        fwrite("},\n", sizeof(char), sizeof("},\n")-1, src);
    }
    fwrite(SrcMappingFooter.data(), 1, SrcMappingFooter.size(), src);
    fclose(src);
}

static void write_header_enum(FILE* header, LanguageDoc const& doc) noexcept
{
    fwrite(HeaderHead.data(), 1, HeaderHead.size(), header);

    fwrite(doc.Key[0].data(), sizeof(doc.Key[0][0]), doc.Key[0].size(), header);

    for(size_t n=1; n < doc.GetRowCount(); ++n)
    {
        fwrite(",\n\t", 1, sizeof(",\n\t") - 1, header);
        fwrite(doc.Key[n].data(), sizeof(doc.Key[0][0]), doc.Key[n].size(), header);
    }

    fwrite(",\n\t", 1, sizeof(",\n\t") - 1, header);
    fwrite("MAX_STRING_COUNT", 1, sizeof("MAX_STRING_COUNT")-1, header);

    fwrite(HeaderFooter.data(), 1, HeaderFooter.size(), header);
    fclose(header);
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Please provide a csv file.\n");
        return -1;
    }

    auto csvFile = argv[1];
    auto header = fopen("OFS_StringsGenerated.h", "wb");
    auto src = fopen("OFS_StringsGenerated.cpp", "wb");
    if(header == nullptr || src == nullptr)
    {
        printf("Failed to create source files.\n");
        return -1;
    }

    try
    {
        LanguageDoc doc{};
        if (auto err = glz::read_file_csv<glz::colwise>(doc, csvFile, std::string{}); err)
        {
            std::printf("Error reading csv file: %.*s, code %u\n", static_cast<int>(err.custom_error_message.size()), err.custom_error_message.data(), err.ec);
            return -1;
        }
        
        std::printf("lines read %lld", doc.GetRowCount());
        write_header_enum(header, doc);
        write_src_file(src, doc);
    }
    catch(const std::exception& e)
    {
        printf("Error: %s\n", e.what());
    }

    return 0;
}