#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>

void get_process()
{
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
        return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if (::Process32First(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == 13624) {
                printf("Found Target\nPID: %u | Threads: %u | PPID: %u | Name: %ws", pe.th32ProcessID, pe.cntThreads, pe.th32ParentProcessID, pe.szExeFile);
            }
        } while (::Process32Next(hSnapshot, &pe));
    }

    ::CloseHandle(hSnapshot);

    return;
}