#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>

// Thin usermode memory layer. Goes through ntdll directly so usermode API
// hooks (some overlays / AV shims install them) don't interfere. This does
// NOT bypass Byfron's handle stripping: if OpenProcess fails with
// ERROR_ACCESS_DENIED the target is protected and you need a kernel read
// primitive (a driver you map yourself) wired into ReadRaw/WriteRaw.

class Memory
{
public:
    Memory() = default;
    ~Memory() { Detach(); }

    bool Attach(const std::wstring& processName);
    void Detach();

    bool IsAttached() const { return m_hProcess != nullptr && m_hProcess != INVALID_HANDLE_VALUE; }
    DWORD Pid() const { return m_pid; }
    uintptr_t Base() const { return m_base; }

    template <typename T>
    T Read(uintptr_t address) const
    {
        T value{};
        ReadRaw(address, &value, sizeof(T));
        return value;
    }

    template <typename T>
    bool Write(uintptr_t address, const T& value) const
    {
        return WriteRaw(address, &value, sizeof(T));
    }

    bool ReadRaw(uintptr_t address, void* buffer, size_t size) const;
    bool WriteRaw(uintptr_t address, const void* buffer, size_t size) const;

    std::string ReadRbxString(uintptr_t container) const;

private:
    using NtReadVirtualMemoryFn  = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, ULONG, PULONG);
    using NtWriteVirtualMemoryFn = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, ULONG, PULONG);

    static DWORD FindPid(const std::wstring& name);
    static uintptr_t FindModuleBase(DWORD pid, const std::wstring& name);

    HANDLE m_hProcess = nullptr;
    DWORD  m_pid = 0;
    uintptr_t m_base = 0;
    std::wstring m_procName;

    NtReadVirtualMemoryFn  m_ntRead  = nullptr;
    NtWriteVirtualMemoryFn m_ntWrite = nullptr;
};
