#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcId(const char* procName)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (_stricmp(procName, procEntry.szExeFile) == 0)
                {
                    procId = procEntry.th32ProcessID;
                    std::cout << procId << " - " << procEntry.szExeFile << " - " << procName << "\n";
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }

    CloseHandle(hSnap);
    return procId;
}

int main()
{
    std::string procName;
    std::cout << "Process Name: ";
    std::cin >> procName;

    std::string dllPath;
    std::cout << "DLL Path: ";
    std::cin >> dllPath;

    DWORD procId = 0;

    while (!procId)
    {
        procId = GetProcId(procName.c_str());
        Sleep(30);
    }

    std::cout << "Found process \"" << procName << "\" (id: " << procId << "!\n";

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, procId);

    if (hProc != INVALID_HANDLE_VALUE || !hProc)
    {
        LPVOID loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if (!loc)
        {
            std::cout << "Could not allocate memory in the specified process.\n";
            return -1;
        }

        WriteProcessMemory(hProc, loc, dllPath.c_str(), MAX_PATH, nullptr);

        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

        if (hThread)
        {
            CloseHandle(hThread);
        }
    }

    if (hProc)
    {
        CloseHandle(hProc);
    }

    return 0;
}