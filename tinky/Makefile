
# Sample makefile

!include <win32.mak>

all: simple.exe challeng.exe

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c

svc: main.obj
  $(link) $(ldebug) $(conflags) -out:svc.exe main.obj $(conlibs) 

