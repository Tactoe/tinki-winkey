
# Sample makefile

!include <win32.mak>

all: svc userToken winkey clear

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c

svc: misc.obj serviceControl.obj
  $(link) $(ldebug) $(conflags) -out:svc.exe misc.obj serviceControl.obj $(conlibs) 

userToken: misc.obj userToken.obj
  $(link) $(ldebug) $(conflags) -out:userToken.exe misc.obj userToken.obj $(conlibs) 

winkey: misc.obj winkey.obj
  $(link) $(ldebug) $(conflags) -out:winkey.exe misc.obj winkey.obj $(conlibs) 

clear:
  del *.obj
  del *.pdb