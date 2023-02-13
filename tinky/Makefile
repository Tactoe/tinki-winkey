
# Sample makefile

!include <win32.mak>

all: svc userToken

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c

svc: serviceControl.obj
  $(link) $(ldebug) $(conflags) -out:svc.exe serviceControl.obj $(conlibs) 

userToken: userToken.obj
  $(link) $(ldebug) $(conflags) -out:userToken.exe userToken.obj $(conlibs) 

