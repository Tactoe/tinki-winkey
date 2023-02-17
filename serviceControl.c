#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SVCNAME "tinky";

//  Forward declarations:
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

SC_HANDLE getService()
{
	TCHAR *serviceName = SVCNAME;
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == NULL)
	{
		printf("Failed to open scmangar");
	}

	SC_HANDLE schService = OpenService(schSCManager, serviceName, SERVICE_ALL_ACCESS);
	CloseServiceHandle(schSCManager);
	if (schService == NULL)
	{
		printf("Failed to open service");

	}
	return schService;
}


void serviceStart()
{
	/* Fetch service */
	SC_HANDLE schService = getService();
	if (schService == NULL) {
		return;
	}

	/* Start service */
	printf("Starting service\n");
	if (StartService(schService, 0, NULL)) {
		printf("Service started succesfully\n");
	}
	else {
		const DWORD lastError = GetLastError();
		if (lastError == ERROR_SERVICE_ALREADY_RUNNING) {
			printf("Service already started\n");
		}
		else {
			printf("Service failed to start (%s)\n", lastError);
		}
	}
	printf("DEAD");

	/* Cleanup handle */
	CloseServiceHandle(schService);
}

void serviceStop()
{
	SC_HANDLE schService;
    SERVICE_STATUS_PROCESS ssStatus; 
	TCHAR *serviceName = SVCNAME;
	DWORD dwBytesNeeded;
	DWORD dwOldCheckPoint; 
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
	SERVICE_STATUS_PROCESS ssp;

	schService = getService();
	if (schService == NULL)
		return;
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

stop_cleanup:
    CloseServiceHandle(schService); 
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
			serviceStart();
		else if (strcmp(argv[1], "stop") == 0)
			serviceStop();
		else if (strcmp(argv[1], "delete") == 0)
			deleteService();
	}

	return 0;
}