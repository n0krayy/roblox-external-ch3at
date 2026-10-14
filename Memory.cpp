#include "Memory.h"
#include <cstring>

bool Memory::Attach(const std::wstring& processName)
{
    Detach();
    m_procName = processName;

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll)
    {
        m_ntRead  = reinterpret_cast<NtReadVirtualMemoryFn>(GetProcAddress(ntdll, "NtReadVirtualMemory"));
        m_ntWrite = reinterpret_cast<NtWriteVirtualMemoryFn>(GetProcAddress(ntdll, "NtWriteVirtualMemory"));
    }

    m_pid = FindPid(processName);
    if (!m_pid)
        return false;

    m_hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
                             PROCESS_QUERY_INFORMATION, FALSE, m_pid);
    if (!m_hProcess || m_hProcess == INVALID_HANDLE_VALUE)
    {
        m_hProcess = nullptr;
        return false; // Byfron strips handles: ERROR_ACCESS_DENIED lands here.
    }

    m_base = FindModuleBase(m_pid, processName);
    return m_base != 0;
}

void Memory::Detach()
{
    if (m_hProcess && m_hProcess != INVALID_HANDLE_VALUE)
        CloseHandle(m_hProcess);
    m_hProcess = nullptr;
    m_pid = 0;
    m_base = 0;
}

bool Memory::ReadRaw(uintptr_t address, void* buffer, size_t size) const
{
    if (!IsAttached() || !address || !buffer || !size)
        return false;
    if (m_ntRead)
    {
        ULONG got = 0;
        return m_ntRead(m_hProcess, reinterpret_cast<PVOID>(address),
                       buffer, static_cast<ULONG>(size), &got) == 0 && got == size;
    }
    SIZE_T got = 0;
    return ReadProcessMemory(m_hProcess, reinterpret_cast<LPCVOID>(address),
                            buffer, size, &got) && got == size;
}

bool Memory::WriteRaw(uintptr_t address, const void* buffer, size_t size) const
{
    if (!IsAttached() || !address || !buffer || !size)
        return false;
    if (m_ntWrite)
    {
        ULONG wrote = 0;
        return m_ntWrite(m_hProcess, reinterpret_cast<PVOID>(address),
                        const_cast<PVOID>(buffer), static_cast<ULONG>(size), &wrote) == 0;
    }
    SIZE_T wrote = 0;
    return WriteProcessMemory(m_hProcess, reinterpret_cast<LPVOID>(address),
                             buffer, size, &wrote) && wrote == size;
}

// Roblox uses its own small-string container: 16 bytes inline capacity,
// heap pointer + length when longer. Container layout we rely on:
//   +0x00 : char inlineBuf[16]  (or heap char* when length >= 16)
//   +0x10 : int32 length
std::string Memory::ReadRbxString(uintptr_t container) const
{
    std::string out;
    if (!container)
        return out;

    int32_t length = Read<int32_t>(container + 0x10);
    if (length <= 0 || length > 256)
        return out;

    char buf[257] = {};
    if (length < 16)
    {
        if (!ReadRaw(container, buf, static_cast<size_t>(length)))
            return {};
    }
    else
    {
        uintptr_t heap = Read<uintptr_t>(container);
        if (!heap || !ReadRaw(heap, buf, static_cast<size_t>(length)))
            return {};
    }
    out.assign(buf, static_cast<size_t>(length));
    return out;
}

DWORD Memory::FindPid(const std::wstring& name)
{
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe))
    {
        do
        {
            if (_wcsicmp(pe.szExeFile, name.c_str()) == 0)
            {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

uintptr_t Memory::FindModuleBase(DWORD pid, const std::wstring& name)
{
    uintptr_t base = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    if (Module32FirstW(snap, &me))
    {
        do
        {
            if (_wcsicmp(me.szModule, name.c_str()) == 0)
            {
                base = reinterpret_cast<uintptr_t>(me.modBaseAddr);
                break;
            }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return base;
}
