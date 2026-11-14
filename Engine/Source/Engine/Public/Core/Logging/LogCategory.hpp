// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "LogVerbosity.hpp"
#include "Core/Memory/Containers/String.hpp"

class RAVEN_API FLogCategory
{
public:
    FLogCategory(const TChar* InName, ELogVerbosity InDefaultVerbosity);
    ~FLogCategory();

    FLogCategory(const FLogCategory&) = delete;
    FLogCategory& operator=(const FLogCategory&) = delete;

public:
    [[nodiscard]] bool8 IsEnabled(const ELogVerbosity Verbosity) const
    {
        return Verbosity <= RuntimeVerbosity.load(std::memory_order_relaxed);
    }

    [[nodiscard]] ELogVerbosity GetVerbosity() const
    {
        return RuntimeVerbosity.load(std::memory_order_relaxed);
    }

    void SetVerbosity(const ELogVerbosity InVerbosity)
    {
        RuntimeVerbosity.store(InVerbosity, std::memory_order_relaxed);
    }

    [[nodiscard]] FStringView GetName() const
    {
        return Name;
    }

    [[nodiscard]] FStringView GetDisplayName() const
    {
        return DisplayName;
    }

public:
    [[nodiscard]] static FLogCategory* Find(FStringView CategoryName);
    static void SetAllVerbosity(ELogVerbosity Verbosity);

private:
    const TChar* Name = nullptr;
    const TChar* DisplayName = nullptr;
    std::atomic<ELogVerbosity> RuntimeVerbosity;

    FLogCategory* Next = nullptr;

    static FLogCategory* ListHead;
};

#define RAVEN_DECLARE_LOG_CATEGORY(CategoryName, DefaultLevel, CompiledLevel) \
    struct FLogCategory_##CategoryName final : FLogCategory \
    { \
        static constexpr ELogVerbosity CompiledVerbosity = ELogVerbosity::CompiledLevel; \
        FLogCategory_##CategoryName() : FLogCategory(#CategoryName, ELogVerbosity::DefaultLevel) {} \
    }; \
    extern FLogCategory_##CategoryName CategoryName

#define RAVEN_DEFINE_LOG_CATEGORY(CategoryName) \
    FLogCategory_##CategoryName CategoryName

#define RAVEN_DEFINE_LOG_CATEGORY_STATIC(CategoryName, DefaultLevel, CompiledLevel) \
    namespace \
    { \
        struct FLogCategory_##CategoryName final : FLogCategory \
        { \
            static constexpr ELogVerbosity CompiledVerbosity = ELogVerbosity::CompiledLevel; \
            FLogCategory_##CategoryName() : FLogCategory(#CategoryName, ELogVerbosity::DefaultLevel) {} \
        }; \
        FLogCategory_##CategoryName CategoryName; \
    } \
    static_assert(true, "")
