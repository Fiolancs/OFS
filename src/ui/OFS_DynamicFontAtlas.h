#pragma once
#include <imgui.h>

#include <string>
#include <string_view>

struct OFS_DynFontAtlas
{
    ImFontGlyphRangesBuilder builder;
    ImVector<ImU32> LastUsedChars;
    ImVector<ImWchar> UsedRanges;
    ImFontConfig config;
    bool checkIfRebuildNeeded = false;
    bool forceRebuild = false;

    static std::string FontOverride;

    static ImFont* DefaultFont;
    static ImFont* DefaultFont2;

    OFS_DynFontAtlas() noexcept;


    static OFS_DynFontAtlas* ptr;
    static void Init() noexcept
    {
        if (ptr != nullptr) return;
        ptr = new OFS_DynFontAtlas();
    }

    static void Shutdown() noexcept
    {
        if (ptr) {
            delete ptr;
            ptr = nullptr;
        }
    }

    // all dynamic which is to be rendered text has to be passed to this function to ensure the glyphs are loaded
    // dynamic text is anything the user inputs but also paths from the file picker dialog or other data from outside
    // text which is known at compile time should be added in the OFS_DynFontAtlas constructor
    static void AddText(std::u8string_view displayedText) noexcept;
    static void Addu8Text(std::string_view displayedText) noexcept; // we are 100% sure the text is already u8
    static void AddTranslationText() noexcept;

    inline static bool NeedsRebuild() noexcept { return ptr->checkIfRebuildNeeded || ptr->forceRebuild; }
    static void RebuildFont(float fontSize) noexcept;
};