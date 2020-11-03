CFLAGS = /Wall /MD /W3 /D_CRT_SECURE_NO_DEPRECATE
 
build : so-cpp.exe
 
so-cpp.exe : so-cpp.obj hashtable.obj
  $(CPP) $(CFLAGS) /Fe$@ $**
 
so-cpp.obj: so-cpp.c
	$(CPP) $(CFLAGS) /c so-cpp.c

hashtable.obj: hashtable.c
	$(CPP) $(CFLAGS) /c hashtable.c

clean : exe_clean obj_clean
 
obj_clean :
  del *.obj
 
exe_clean :
  del so-cpp.exe
