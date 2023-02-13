
# Sample makefile

!include <win32.mak>

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c

winkey: winkey.obj
  $(link) $(ldebug) $(conflags) -out:winkey.exe winkey.obj $(conlibs) 
