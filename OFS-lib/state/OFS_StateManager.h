#pragma once
#include "OFS_Util.h"
#include "OFS_Serialization.h"

#include <any>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <string_view>

struct OFS_StateMetadata
{
public:
    template <typename T>
    static OFS_StateMetadata CreateMetadata(std::string_view typeName) noexcept
    {
        // QQQ
        return OFS_StateMetadata{
            typeName,
            &OFS_StateMetadata::createUntyped<T>,
            //.serializer = &OFS_StateMetadata::serializeUntyped<T>,
            //.deserializer = &OFS_StateMetadata::deserializeUntyped<T>,
        };
    }
    
    std::any          create(void) const noexcept { return creator(); }
    std::string_view getName(void) const noexcept { return name; }
    
    //bool Serialize(const std::any& value, nlohmann::json& obj, bool enableBinary) const noexcept {
    //    return serializer(value, obj, enableBinary);
    //}
    //
    //bool Deserialize(std::any& value, const nlohmann::json& obj, bool enableBinary) const noexcept {
    //    return deserializer(value, obj, enableBinary);
    //}

private:
    using OFS_StateCreator = std::any(*)() noexcept;
    //using OFS_StateSerializer = bool (*)(const std::any&, nlohmann::json&, bool) noexcept;
    //using OFS_StateDeserializer = bool (*)(std::any&, const nlohmann::json&, bool) noexcept;

    std::string_view name;
    OFS_StateCreator creator;
    //OFS_StateSerializer serializer;
    //OFS_StateDeserializer deserializer;

    template <typename T>
    static std::any createUntyped() noexcept
    {
        T instance{};
        //for_each(refl::reflect<T>().members, [&](auto member) {
        //    if constexpr (refl::descriptor::has_attribute<UiProperty>(member)) {
        //        auto&& prop = refl::descriptor::get_attribute<UiProperty>(member);
        //        if (auto propIter = props.find(member.name.str()); propIter != props.end()) {
        //            member(instance) = prop.parser(propIter->second);
        //        }
        //    }
        //});
        return T{};
    }

    //template<typename T>
    //static bool serializeUntyped(const std::any& value, nlohmann::json& obj, bool enableBinary) noexcept
    //{
    //    // QQQ
    //    //auto& realValue = std::any_cast<const T&>(value);
    //    //return enableBinary ? OFS::Serializer<true>::Serialize(realValue, obj) : OFS::Serializer<false>::Serialize(realValue, obj);
    //    return true;
    //}
    //
    //template<typename T>
    //static bool deserializeUntyped(std::any& value, const nlohmann::json& obj, bool enableBinary) noexcept
    //{
    //    // QQQ
    //    //auto& realValue = std::any_cast<T&>(value);
    //    //return enableBinary ? OFS::Serializer<true>::Deserialize(realValue, obj) : OFS::Serializer<false>::Deserialize(realValue, obj);
    //    return true;
    //}

    OFS_StateMetadata(std::string_view typeName, OFS_StateCreator ctor)
        : name{ typeName }, creator{ ctor }
    {}
};

class OFS_StateRegistry
{
public:
    static OFS_StateRegistry& Get() noexcept
    {
        static OFS_StateRegistry instance;
        return instance;
    }

    const OFS_StateMetadata* Find(std::string_view typeName) const noexcept
    {
        auto iter = std::find_if(metadata.begin(), metadata.end(), [&](auto&& x) {
            return x.getName() == typeName;
        });
        if (iter != metadata.end()) {
            return &(*iter);
        }
        return nullptr;
    }

    template <typename T>
    void RegisterState(std::string_view typeName)
    {
        metadata.push_back(OFS_StateMetadata::CreateMetadata<T>(typeName));
    }

private:
    std::vector<OFS_StateMetadata> metadata;
    OFS_StateRegistry() noexcept {}
};

#define OFS_REGISTER_STATE(StateTypeName) OFS_StateRegistry::Get().RegisterState<StateTypeName>(#StateTypeName)

struct OFS_State
{
    std::string_view stateName;
    std::string_view typeName;
    const OFS_StateMetadata* Metadata = nullptr;
    std::any State;
};

class OFS_StateManager
{
public:                        
    using StateHandleMap = std::map<std::string_view, std::pair<std::string_view, uint32_t>>; // StateName -> Pair(TypeName, Handle)
    inline static constexpr std::uint32_t INVALID_ID = 0xFFFF'FFFF;

    template <typename T>
    inline static std::uint32_t registerState(std::string_view stateName, std::string_view typeName, std::vector<OFS_State>& stateCollection, StateHandleMap& handleMap) noexcept
    {
        auto it = handleMap.find(stateName);
        if(it == handleMap.end())
        {
            LOGF_DEBUG("Registering new state \"{:s}\". Type: {:s}", stateName, typeName);
        
            auto metadata = OFS_StateRegistry::Get().Find(typeName);
            FUN_ASSERT(metadata, "State wasn't registered using OFS_REGISTER_STATE macro");
        
            std::uint32_t id = static_cast<std::uint32_t>( stateCollection.size() );
            stateCollection.emplace_back(OFS_State{ stateName, typeName, metadata, std::make_any<T>() });
        
            handleMap.try_emplace(handleMap.end(), stateName, std::make_pair(typeName, id));
            return id;
        }
        else
        {
            FUN_ASSERT(stateCollection[it->second.second].stateName == stateName, "Something went wrong");
            LOGF_DEBUG("Loading existing state \"%s\"", stateName);
            return it->second.second;
        }

        return INVALID_ID;
    }

    template <typename T>
    inline static T& getState(uint32_t id, std::vector<OFS_State>& stateCollection) noexcept
    {
        FUN_ASSERT(id < stateCollection.size(), "out of bounds");
        auto& item = stateCollection[id];
        auto& value = std::any_cast<T&>(item.State);
        return value;
    }

    public:
    static void Init() noexcept;
    static void Shutdown() noexcept;
    inline static OFS_StateManager* Get() noexcept { return instance; }

    template<typename T>
    inline uint32_t RegisterApp(std::string_view name, std::string_view typeName) noexcept
    {
        return registerState<T>(name, typeName, ApplicationState, ApplicationHandleMap);
    }

    template <typename T>
    inline uint32_t RegisterProject(std::string_view name, std::string_view typeName) noexcept
    {
        return registerState<T>(name, typeName, ProjectState, ProjectHandleMap);
    }

    template<typename T>
    inline T& GetApp(uint32_t id) noexcept
    {
        return getState<T>(id, ApplicationState);
    }

    template<typename T>
    inline T& GetProject(uint32_t id) noexcept
    {
        return getState<T>(id, ProjectState);
    }

    void SerializeAppAll(bool enableBinary) noexcept;
    bool DeserializeAppAll(/*const nlohmann::json& state, */bool enableBinary) noexcept;

    void SerializeProjectAll(bool enableBinary) noexcept;
    bool DeserializeProjectAll(/*const nlohmann::json& project, */bool enableBinary) noexcept;
    void ClearProjectAll() noexcept;

private:
    std::vector<OFS_State> ApplicationState;
    std::vector<OFS_State> ProjectState;

    StateHandleMap ApplicationHandleMap;
    StateHandleMap ProjectHandleMap;

    static OFS_StateManager* instance;
};
