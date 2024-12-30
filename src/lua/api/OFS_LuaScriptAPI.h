#pragma once
#include "lua/OFS_Lua.h"
#include "OFS_Util.h"
#include "Funscript/Funscript.h"
#include "Funscript/FunscriptAction.h"

#include <set>
#include <tuple>
#include <memory>
#include <vector>
#include <optional>
#include <filesystem>

struct LuaFunscriptAction
{
    using lua_Integer = int;
    using lua_Number  = double;
    FunscriptAction o;
    bool selected = false;

    LuaFunscriptAction(FunscriptAction action, bool selected) noexcept
        : o(action), selected(selected) 
    {}
    LuaFunscriptAction(lua_Number at, lua_Integer pos) noexcept
    {
        o.atS = std::max(0.0, at);
        o.pos = Util::Clamp<lua_Integer>(pos, 0, 100);
    }
    LuaFunscriptAction(lua_Number at, lua_Integer pos, bool selected) 
        : LuaFunscriptAction(at, pos)
    {
        this->selected = selected;
    }

    inline lua_Number at() noexcept
    {
        return o.atS;
    }

    inline void set_at(lua_Number at) noexcept
    {
        o.atS = std::max(0.0, at);
    }

    inline lua_Integer pos() noexcept
    {
        return o.pos;
    }

    inline void set_pos(lua_Integer pos) noexcept
    {            
        o.pos = Util::Clamp<lua_Integer>(pos, 0, 100);
    }
};

using LuaFunscriptArray = std::vector<LuaFunscriptAction>;

class LuaFunscript
{
    using lua_Integer = int;
    using lua_Number = double;
private:
        int32_t scriptIdx = -1;
        std::weak_ptr<Funscript> script;
        LuaFunscriptArray actions;
        std::set<uint32_t> markedIndices;
public:
        LuaFunscript(int32_t scriptIdx, std::weak_ptr<Funscript> script) noexcept;
        LuaFunscript(const FunscriptArray& actions) noexcept;

        inline void TakeSnapshot() noexcept
        {
            OFS_PROFILE(__FUNCTION__);
            auto ref = script.lock();
            if(ref) {
                auto size = ref->Actions().size();
                actions.reserve(size);
                for(auto action : ref->Actions()) {
                    actions.emplace_back(action, ref->IsSelected(action));
                }
            }
        }

        inline LuaFunscriptArray& Actions() noexcept 
        {
            return actions;
        }

        inline void Sort() noexcept
        {
            std::stable_sort(actions.begin(), actions.end(),
                [](auto a1, auto a2) {
                    return a1.o.atS < a2.o.atS;
                });
        }

        void Commit(/*sol::this_state L*/) noexcept;
        bool HasSelection() const noexcept;
        std::vector<lua_Integer> SelectedIndices() const noexcept;

        std::filesystem::path Path() const noexcept;
        const char* Name() const noexcept;

        std::optional<std::tuple<LuaFunscriptAction, lua_Integer>> ClosestAction(lua_Number time) noexcept;
        std::optional<std::tuple<LuaFunscriptAction, lua_Integer>> ClosestActionAfter(lua_Number time) noexcept;
        std::optional<std::tuple<LuaFunscriptAction, lua_Integer>> ClosestActionBefore(lua_Number time) noexcept;

        void MarkForRemoval(lua_Integer actionIdx/*, sol::this_state L*/) noexcept;
        lua_Integer RemoveMarked() noexcept;
};

class OFS_ScriptAPI
{
    using lua_Integer = int;
private:
        static std::unique_ptr<LuaFunscript> Script(lua_Integer idx) noexcept;
        static lua_Integer ActiveIdx() noexcept;
        static bool Undo() noexcept;

        static std::unique_ptr<LuaFunscript> Clipboard() noexcept;
public:
        OFS_ScriptAPI(/*sol::usertype<class OFS_ExtensionAPI>& ofs*/) noexcept;
};