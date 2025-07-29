#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

DWORD getProcessId(const wchar_t* targetProcessName) {
    PROCESSENTRY32W processEntry = { sizeof(PROCESSENTRY32W) };
    HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshotHandle == INVALID_HANDLE_VALUE) return 0;

    if (Process32FirstW(snapshotHandle, &processEntry)) {
        do {
            if (_wcsicmp(processEntry.szExeFile, targetProcessName) == 0) {
                DWORD pid = processEntry.th32ProcessID;
                CloseHandle(snapshotHandle);
                return pid;
            }
        } while (Process32NextW(snapshotHandle, &processEntry));
    }

    CloseHandle(snapshotHandle);
    return 0;
}

BOOL inject(DWORD processId, const char* dllFullPath) {
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!processHandle) return FALSE;

    SIZE_T dllPathLength = strlen(dllFullPath) + 1;
    LPVOID remoteMemory = VirtualAllocEx(processHandle, NULL, dllPathLength, MEM_COMMIT, PAGE_READWRITE);
    if (!remoteMemory) {
        CloseHandle(processHandle);
        return FALSE;
    }

    WriteProcessMemory(processHandle, remoteMemory, dllFullPath, dllPathLength, NULL);

    LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    HANDLE remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, remoteMemory, 0, NULL);

    if (!remoteThread) {
        VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return FALSE;
    }

    WaitForSingleObject(remoteThread, INFINITE);
    VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(remoteThread);
    CloseHandle(processHandle);
    return TRUE;
}

int main() {
    char fullDllPath[MAX_PATH];
    GetModuleFileNameA(NULL, fullDllPath, MAX_PATH);
    PathRemoveFileSpecA(fullDllPath);
    PathCombineA(fullDllPath, fullDllPath, "logger.dll");

    DWORD targetProcessId = getProcessId(L"notepad.exe");
    if (!targetProcessId || !inject(targetProcessId, fullDllPath)) {
        printf("DLL injection failed! Error: %lu\n", GetLastError());
        return 1;
    }

    printf("Successfully injected %s into process ID %lu\n", fullDllPath, targetProcessId);
    return 0;
}
