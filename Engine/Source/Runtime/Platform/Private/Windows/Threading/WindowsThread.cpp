// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Windows/Threading/WindowsThread.hpp"

#include <process.h>

#include "CoreAssertions.hpp"

#include "Memory/Memory.hpp"

#include "Profiling/Profiling.hpp"

#include "Utility/Atomic.hpp"

struct FThreadLaunchData
{
    FThreadEntryFunction EntryFunction;
    void* Data;
    const TChar* ThreadName = "UnnamedThread";
    TAtomic<bool8> bIsStarted = false;
};

unsigned __stdcall ThreadEntry(void* Parameter)
{
    FThreadLaunchData* LaunchData = static_cast<FThreadLaunchData*>(Parameter);
    RAVEN_PROFILE_THREAD(LaunchData->ThreadName);

    // Copy entry function, because launch data will be deleted after thread creation
    const FThreadEntryFunction EntryFunction = LaunchData->EntryFunction;
    void* Data = LaunchData->Data;

    LaunchData->bIsStarted.store(true, std::memory_order_relaxed);
    LaunchData->bIsStarted.notify_one();
    EntryFunction(Data);
    return 0;
}

int ToWin32Priority(const EThreadPriority Priority)
{
    switch (Priority)
    {
    case EThreadPriority::Low: return THREAD_PRIORITY_LOWEST;
    case EThreadPriority::Normal: return THREAD_PRIORITY_NORMAL;
    case EThreadPriority::High: return THREAD_PRIORITY_HIGHEST;
    case EThreadPriority::TimeCritical: return THREAD_PRIORITY_TIME_CRITICAL;
    }
    RAVEN_CORE_ASSERT(false, "Unhandled EThreadPriority");
    return THREAD_PRIORITY_NORMAL;
}

FWindowsThread::FWindowsThread(const FThreadDesc& InThreadDesc)
    : ThreadDesc(InThreadDesc)
{
    FThreadLaunchData* LaunchData = Memory::New<FThreadLaunchData>(ThreadDesc.EntryFunction, ThreadDesc.Data, ThreadDesc.Name);
    ThreadHandle = reinterpret_cast<void*>(_beginthreadex(nullptr, static_cast<unsigned>(ThreadDesc.StackSize), ThreadEntry, LaunchData, CREATE_SUSPENDED, reinterpret_cast<unsigned*>(&ThreadId)));
    RAVEN_CORE_ASSERT(ThreadHandle != nullptr, "Failed to create thread");

    wchar_t WideName[64];
    MultiByteToWideChar(CP_UTF8, 0, ThreadDesc.Name, -1, WideName, 64);
    SetThreadDescription(ThreadHandle, WideName);
    SetThreadPriority(ThreadHandle, ToWin32Priority(ThreadDesc.Priority));
    if (ThreadDesc.AffinityMask != 0)
    {
        SetThreadAffinityMask(ThreadHandle, ThreadDesc.AffinityMask);
    }

    ResumeThread(ThreadHandle);
    LaunchData->bIsStarted.wait(false, std::memory_order_acquire);
    Memory::Delete(LaunchData);
}

FWindowsThread::FWindowsThread(FWindowsThread&& Other) noexcept
{
    ThreadHandle = Other.ThreadHandle;
    ThreadId = Other.ThreadId;
    ThreadDesc = Other.ThreadDesc;
    Other.ThreadHandle = INVALID_HANDLE_VALUE;
    Other.ThreadId = 0;
    Other.ThreadDesc = {};
}

FWindowsThread::~FWindowsThread()
{
    if (ThreadHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(ThreadHandle);
    }
    ThreadHandle = INVALID_HANDLE_VALUE;
}

FWindowsThread& FWindowsThread::operator=(FWindowsThread&& Other) noexcept
{
    if (this != &Other)
    {
        if (ThreadHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(ThreadHandle);
        }
        ThreadHandle = Other.ThreadHandle;
        ThreadId = Other.ThreadId;
        ThreadDesc = Other.ThreadDesc;
        Other.ThreadHandle = INVALID_HANDLE_VALUE;
        Other.ThreadId = 0;
        Other.ThreadDesc = {};
    }
    return *this;
}

bool8 FWindowsThread::IsValid() const
{
    return ThreadHandle != INVALID_HANDLE_VALUE && ThreadHandle != nullptr;
}

bool8 FWindowsThread::IsJoinable() const
{
    return IsValid();
}

uint64 FWindowsThread::GetThreadId() const
{
    return ThreadId;
}

void FWindowsThread::Join()
{
    RAVEN_CORE_ASSERT(IsJoinable(), "Thread is not joinable");
    WaitForSingleObject(ThreadHandle, INFINITE);
    CloseHandle(ThreadHandle);
    ThreadHandle = INVALID_HANDLE_VALUE;
}

void FWindowsThread::Detach()
{
    RAVEN_CORE_ASSERT(IsJoinable(), "Thread is not joinable");
    CloseHandle(ThreadHandle);
    ThreadHandle = INVALID_HANDLE_VALUE;
}
