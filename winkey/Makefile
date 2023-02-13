
# Sample makefile

!include <win32.mak>

all: simple.exe challeng.exe

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c

winkey: main.obj
  $(link) $(ldebug) $(conflags) -out:winkey.exe main.obj $(conlibs) 


hooks: hooks.obj
  $(link) $(ldebug) $(conflags) -out:hooks.exe hooks.obj $(conlibs) 
