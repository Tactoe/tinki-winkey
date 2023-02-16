#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SVCNAME "tinky";

//  Forward declarations:
BOOL GetProcessList( );
BOOL ListProcessModules( DWORD dwPID );
BOOL ListProcessThreads( DWORD dwOwnerPID );
void printError( TCHAR* msg );
int getFolderStringLength (TCHAR* str);

void installService()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
    TCHAR modulePath[MAX_PATH];
    TCHAR moduleFolder[MAX_PATH];
    TCHAR servicePath[MAX_PATH];

    if(!GetModuleFileName(NULL, modulePath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }
    strncpy_s(moduleFolder, MAX_PATH, modulePath, getFolderStringLength(modulePath));
    snprintf(servicePath, MAX_PATH, "%s\\winkey.exe", moduleFolder);

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

void deleteService()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
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

void toggleService(BOOL startService)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
    SERVICE_STATUS_PROCESS ssStatus; 
	TCHAR *serviceName = SVCNAME;
	DWORD dwBytesNeeded;
	DWORD dwOldCheckPoint; 
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
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

		if (!QueryServiceStatusEx( 
            schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // size needed if buffer is too small
	    {
	        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
	        CloseServiceHandle(schService); 
	        CloseServiceHandle(schSCManager);
	        return; 
	    }

	    // Check if the service is already running. It would be possible 
	    // to stop the service here, but for simplicity this example just returns. 

	    if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	    {
	        printf("Cannot start the service because it is already running\n");
	        CloseServiceHandle(schService); 
	        CloseServiceHandle(schSCManager);
	        return; 
	    }

	    // Save the tick count and initial checkpoint.

	    dwStartTickCount = GetTickCount();
	    dwOldCheckPoint = ssStatus.dwCheckPoint;

	    // Wait for the service to stop before attempting to start it.

	    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	    {
	        // Do not wait longer than the wait hint. A good interval is 
	        // one-tenth of the wait hint but not less than 1 second  
	        // and not more than 10 seconds. 
	 
	        dwWaitTime = ssStatus.dwWaitHint / 10;

	        if( dwWaitTime < 1000 )
	            dwWaitTime = 1000;
	        else if ( dwWaitTime > 10000 )
	            dwWaitTime = 10000;

	        Sleep( dwWaitTime );

	        // Check the status until the service is no longer stop pending. 
	 
	        if (!QueryServiceStatusEx( 
	                schService,                     // handle to service 
	                SC_STATUS_PROCESS_INFO,         // information level
	                (LPBYTE) &ssStatus,             // address of structure
	                sizeof(SERVICE_STATUS_PROCESS), // size of structure
	                &dwBytesNeeded ) )              // size needed if buffer is too small
	        {
	            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
	            CloseServiceHandle(schService); 
	            CloseServiceHandle(schSCManager);
	            return; 
	        }

	        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
	        {
	            // Continue to wait and check.

	            dwStartTickCount = GetTickCount();
	            dwOldCheckPoint = ssStatus.dwCheckPoint;
	        }
	        else
	        {
	            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
	            {
	                printf("Timeout waiting for service to stop\n");
	                CloseServiceHandle(schService); 
	                CloseServiceHandle(schSCManager);
	                return; 
	            }
	        }
	    }
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