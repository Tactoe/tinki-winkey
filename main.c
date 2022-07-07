#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

//  Forward declarations:
BOOL GetProcessList( );
BOOL ListProcessModules( DWORD dwPID );
BOOL ListProcessThreads( DWORD dwOwnerPID );
void printError( TCHAR* msg );

// DWORD : type d'int utilise pour PID sur win


BOOL SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	BOOL bEnablePrivilege   // to enable or disable privilege
)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid))        // receives LUID of privilege
	{
		printf("[-] LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		printf("[-] AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		printf("[-] The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}

int main()
{

    HANDLE WinlogonHandle;
    HANDLE TokenHandle;

    DWORD PID_TO_IMPERSONATE = 920;

    HANDLE currentTokenHandle = NULL;
	BOOL getCurrentToken = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &currentTokenHandle);
	if (SetPrivilege(currentTokenHandle, L"SeDebugPrivilege", TRUE))
	{
		printf("[+] SeDebugPrivilege enabled!\n");
	}

    // WinlogonHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 920);
    // if (OpenProcessToken(WinlogonHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
    // {
    //      printf("SCURREEES\n");
    //     // ImpersonateLoggedOnUser(TokenHandle);
    //     CloseHandle(TokenHandle);
         
    // }
    // else
    // {
    //     printf("OpenProcessToken failed: (%d)\n", GetLastError());
    // }

    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, PID_TO_IMPERSONATE);
	if (GetLastError() == NULL)
		printf("[+] OpenProcess() success!\n");
	else
	{
		printf("[-] OpenProcess() Return Code: %i\n", processHandle);
		printf("[-] OpenProcess() Error: %i\n", GetLastError());
	}
	
	// Call OpenProcessToken(), print return code and error code
	BOOL getToken = OpenProcessToken(processHandle, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &WinlogonHandle);
	if (GetLastError() == NULL)
		printf("[+] OpenProcessToken() success!\n");
	else
	{
		printf("[-] OpenProcessToken() Return Code: %i\n", getToken);
		printf("[-] OpenProcessToken() Error: %i\n", GetLastError());
	}

    CloseHandle(WinlogonHandle);
    // Get a handle to the SCM database. 
 
    // schSCManager =OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    // schSCManager = OpenSCManager( 
    //     NULL,                    // local computer
    //     NULL,                    // ServicesActive database 
    //     SC_MANAGER_ALL_ACCESS);  // full access rights 

    // if (NULL == schSCManager) 
    // {
    //     printf("OpenSCManager failed (%d)\n", GetLastError());
    //     return;
    // }
    // schService = OpenService( 
    //     schSCManager,            // SCM database 
    //     "winlogon.exe",               // name of service 
    //     SERVICE_CHANGE_CONFIG);  // need change config access 
 
    // if (schService == NULL)
    // { 
    //     printf("OpenService failed (%d)\n", GetLastError()); 
    //     CloseServiceHandle(schSCManager);
    //     return;
    // }    
    //GetProcessList();
    return 0;
}