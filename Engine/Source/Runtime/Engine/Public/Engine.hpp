// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <type_traits>

#include "CoreTypes.hpp"

#include "Error/Result.hpp"

#include "Memory/Memory.hpp"
#include "Memory/Containers/Map.hpp"
#include "Memory/Containers/String.hpp"
#include "Memory/Containers/Vector.hpp"

#include "Utility/Hashing.hpp"

class ISubsystem;

struct FSubsystemEntry
{
    uint64 TypeId = 0;
    ISubsystem* Subsystem = nullptr;
    FStringView Name;
};

class ENGINE_API FEngine
{
public:
    FEngine() = default;
    ~FEngine() = default;

public:
    [[nodiscard]] TResult<void> Initialize();
    void Run();
    void Shutdown();

    void RequestExit() noexcept;

    [[nodiscard]] bool8 IsExitRequested() const noexcept;

public:
    template <typename T, typename... TArguments> requires std::is_base_of_v<ISubsystem, T> && std::is_constructible_v<T, TArguments...>
    void AddSubsystem(TArguments&&... Arguments)
    {
        if (SubsystemLookupMap.contains(Hash::TypeNameHash<T>()))
        {
            return;
        }
        T* Subsystem = Memory::New<T>(std::forward<TArguments>(Arguments)...);
        FSubsystemEntry Entry = {
            .TypeId = Hash::TypeNameHash<T>(),
            .Subsystem = Subsystem,
            .Name = Hash::TypeName<T>()
        };
        Subsystems.push_back(Entry);
        SubsystemLookupMap.emplace(Entry.TypeId, Subsystem);
    }

    template <typename T>
    [[nodiscard]] T* GetSubsystem()
    {
        if (!SubsystemLookupMap.contains(Hash::TypeNameHash<T>()))
        {
            return nullptr;
        }
        return static_cast<T*>(SubsystemLookupMap.at(Hash::TypeNameHash<T>()));
    }

protected:
    TVector<FSubsystemEntry> Subsystems;
    TUnorderedMap<uint64, ISubsystem*> SubsystemLookupMap;

    bool8 bIsInitialized = false;
    bool8 bIsRunning = false;
    bool8 bIsExitRequested = false;
};

[[nodiscard]] ENGINE_API FEngine* GetEngine();
ENGINE_API void DestroyEngine();
