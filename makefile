# Sample makefile

!include <win32.mak>

all: simple.exe challeng.exe

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c

tinky: main.obj
  $(link) $(ldebug) $(conflags) -out:tinky.exe main.obj $(conlibs) 

