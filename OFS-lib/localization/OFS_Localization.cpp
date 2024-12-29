#include "OFS_Localization.h"
#include "OFS_Util.h"

#include <glaze/csv.hpp>

#include <array>
#include <optional>
#include <filesystem>


OFS_Translator* OFS_Translator::ptr = nullptr;

OFS_Translator::OFS_Translator(std::filesystem::path translationDir) noexcept
    : stdPath{ translationDir / TranslationDir }
{
    OFS::util::createDirectories(stdPath);
    // initialize with the default strings
    LoadDefaults();
}

void OFS_Translator::LoadDefaults() noexcept
{
    StringData = std::vector<char>();
    std::memcpy(Translation.data(), OFS_DefaultStrings::Default.data(), Translation.size() * sizeof(const char*));
}

struct LanguageDoc
{
    std::vector<std::string> Key;
    std::vector<std::string> Default;
    std::vector<std::string> Translation;

    auto GetRow(std::size_t n) { return std::forward_as_tuple(Key[n], Default[n], Translation[n]); }
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

static std::optional<LanguageDoc> OpenDocument(std::filesystem::path path) noexcept
{
    if (std::string file_contents = OFS::util::readFileString(path); !file_contents.empty())
    {
        LanguageDoc doc{};
        if (auto const err = glz::read_csv<glz::colwise>(doc, file_contents); !err)
        {
            return doc;
        }
        else
        {
            LOG_ERROR(err.custom_error_message);
        }
    }

    return {};
}

bool OFS_Translator::LoadTranslation(const char* name) noexcept
{
    auto docOpt = OpenDocument(stdPath / OFS::util::utf8ToUtf16(name));
    if(!docOpt.has_value()) return false;

    auto doc = docOpt.value();
    std::vector<char> tmpData;
    tmpData.reserve(1024*15);

    std::vector<const char*> tmpTranslation;
    tmpTranslation.resize(OFS_DefaultStrings::Default.size());
    
    std::vector<int> tmpTrIndices;
    tmpTrIndices.resize(OFS_DefaultStrings::Default.size(), -1);

    for(size_t i = 0; i < doc.GetRowCount(); ++i)
    {
        auto row = doc.GetRow(i);

        auto& key   = std::get<0>(row);
        auto  value = std::get<2>(row);
        value = OFS::util::trim(value);

        if(value.empty()) {
            continue;
        }

        auto it = OFS_DefaultStrings::KeyMapping.find(key);
        if(it != OFS_DefaultStrings::KeyMapping.end())
        {
            size_t index = static_cast<size_t>(it->second);
            size_t valPos = tmpData.size();
            tmpTrIndices[index] = valPos;

            for(auto c : value) {
                tmpData.emplace_back(c);
            }
            tmpData.emplace_back('\0');
        }
    }

    for(int i=0; i < OFS_DefaultStrings::Default.size(); i += 1)
    {
        if(tmpTrIndices[i] >= 0) {
            tmpTranslation[i] = &tmpData[tmpTrIndices[i]];
        }
        else 
        {
            tmpTranslation[i] = OFS_DefaultStrings::Default[i];
        }
    }

    StringData = std::move(tmpData);
    memcpy(Translation.data(), tmpTranslation.data(), Translation.size() * sizeof(const char*));
    return true;
}

bool OFS_Translator::MergeIntoOne(const char* inputPath1, const char* inputPath2, const char* outputPath) noexcept
{
    /*
        This function takes input1 and merges input2 into it.
        The merged csv gets written to outputPath.
    */
    auto inputOpt1 = OpenDocument(inputPath1);
    auto inputOpt2 = OpenDocument(inputPath2);
    if(!inputOpt1.has_value() || !inputOpt2.has_value()) return false;
    auto input1 = std::move(inputOpt1.value());
    auto input2 = std::move(inputOpt2.value());
        
    std::array<const char*, static_cast<int>(Tr::MAX_STRING_COUNT)> lut;
    for(auto& mapping : OFS_DefaultStrings::KeyMapping) {
        lut[static_cast<int>(mapping.second)] = mapping.first.c_str();
    }

    std::array<std::string, static_cast<int>(Tr::MAX_STRING_COUNT)> input1Lut;
    for(size_t i=0, size=input1.GetRowCount(); i < size; i += 1) {
        auto row = input1.GetRow(i);
        auto it = OFS_DefaultStrings::KeyMapping.find(std::get<0>(row));
        if(it != OFS_DefaultStrings::KeyMapping.end()) {
            input1Lut[static_cast<int>(it->second)] = std::get<2>(row);
        }
    }

    std::array<std::string, static_cast<int>(Tr::MAX_STRING_COUNT)> input2Lut;
    for(size_t i=0, size=input2.GetRowCount(); i < size; i += 1) {
        auto row = input2.GetRow(i);
        auto it = OFS_DefaultStrings::KeyMapping.find(std::get<0>(row));
        if(it != OFS_DefaultStrings::KeyMapping.end()) {
            input2Lut[static_cast<int>(it->second)] = std::get<2>(row);
        }
    }

    LanguageDoc out{};
    out.Key.resize(static_cast<int>(Tr::MAX_STRING_COUNT));
    out.Default.resize(static_cast<int>(Tr::MAX_STRING_COUNT));
    out.Translation.resize(static_cast<int>(Tr::MAX_STRING_COUNT));

    for(int idx = 0; idx < static_cast<int>(Tr::MAX_STRING_COUNT); ++idx) {
        Tr current = static_cast<Tr>(idx);
        out.Key[idx]         = lut[idx];
        out.Default[idx]     = TRD(current);
        out.Translation[idx] = input1Lut[idx].empty() ? input2Lut[idx] : input1Lut[idx];
    }

    if (auto err = glz::write_file_csv(out, outputPath, std::string{}); err)
    {
        LOG_ERROR(err.custom_error_message);
        return false;
    }
    return true;
}
