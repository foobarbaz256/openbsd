# Microsoft Developer Studio Generated NMAKE File, Based on ApacheModuleSpeling.dsp
!IF "$(CFG)" == ""
CFG=ApacheModuleSpeling - Win32 Release
!MESSAGE No configuration specified. Defaulting to ApacheModuleSpeling - Win32\
 Release.
!ENDIF 

!IF "$(CFG)" != "ApacheModuleSpeling - Win32 Release" && "$(CFG)" !=\
 "ApacheModuleSpeling - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ApacheModuleSpeling.mak"\
 CFG="ApacheModuleSpeling - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ApacheModuleSpeling - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ApacheModuleSpeling - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ApacheModuleSpeling - Win32 Release"

OUTDIR=.\ApacheModuleSpelingR
INTDIR=.\ApacheModuleSpelingR
# Begin Custom Macros
OutDir=.\.\ApacheModuleSpelingR
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\ApacheModuleSpeling.dll"

!ELSE 

ALL : "$(OUTDIR)\ApacheModuleSpeling.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\mod_speling.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.dll"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.exp"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "SHARED_MODULE" /Fp"$(INTDIR)\ApacheModuleSpeling.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\ApacheModuleSpelingR/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ApacheModuleSpeling.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\CoreR\ApacheCore.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows\
 /dll /incremental:no /pdb:"$(OUTDIR)\ApacheModuleSpeling.pdb" /machine:I386\
 /out:"$(OUTDIR)\ApacheModuleSpeling.dll"\
 /implib:"$(OUTDIR)\ApacheModuleSpeling.lib" 
LINK32_OBJS= \
	"$(INTDIR)\mod_speling.obj"

"$(OUTDIR)\ApacheModuleSpeling.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ApacheModuleSpeling - Win32 Debug"

OUTDIR=.\ApacheModuleSpelingD
INTDIR=.\ApacheModuleSpelingD
# Begin Custom Macros
OutDir=.\.\ApacheModuleSpelingD
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\ApacheModuleSpeling.dll"

!ELSE 

ALL : "$(OUTDIR)\ApacheModuleSpeling.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\mod_speling.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.dll"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.exp"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.ilk"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.lib"
	-@erase "$(OUTDIR)\ApacheModuleSpeling.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "SHARED_MODULE" /Fp"$(INTDIR)\ApacheModuleSpeling.pch"\
 /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\ApacheModuleSpelingD/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ApacheModuleSpeling.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\CoreD\ApacheCore.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows\
 /dll /incremental:yes /pdb:"$(OUTDIR)\ApacheModuleSpeling.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)\ApacheModuleSpeling.dll"\
 /implib:"$(OUTDIR)\ApacheModuleSpeling.lib" 
LINK32_OBJS= \
	"$(INTDIR)\mod_speling.obj"

"$(OUTDIR)\ApacheModuleSpeling.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "ApacheModuleSpeling - Win32 Release" || "$(CFG)" ==\
 "ApacheModuleSpeling - Win32 Debug"
SOURCE=..\..\modules\standard\mod_speling.c

!IF  "$(CFG)" == "ApacheModuleSpeling - Win32 Release"

DEP_CPP_MOD_S=\
	"..\..\include\alloc.h"\
	"..\..\include\ap.h"\
	"..\..\include\ap_mmn.h"\
	"..\..\include\buff.h"\
	"..\..\include\conf.h"\
	"..\..\include\hsregex.h"\
	"..\..\include\http_config.h"\
	"..\..\include\http_core.h"\
	"..\..\include\http_log.h"\
	"..\..\include\httpd.h"\
	"..\..\include\util_uri.h"\
	".\os.h"\
	".\readdir.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_MOD_S=\
	"..\..\include\ebcdic.h"\
	"..\..\include\os.h"\
	"..\..\include\sfio.h"\
	

"$(INTDIR)\mod_speling.obj" : $(SOURCE) $(DEP_CPP_MOD_S) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ApacheModuleSpeling - Win32 Debug"

DEP_CPP_MOD_S=\
	"..\..\include\alloc.h"\
	"..\..\include\ap.h"\
	"..\..\include\ap_mmn.h"\
	"..\..\include\buff.h"\
	"..\..\include\conf.h"\
	"..\..\include\hsregex.h"\
	"..\..\include\http_config.h"\
	"..\..\include\http_core.h"\
	"..\..\include\http_log.h"\
	"..\..\include\httpd.h"\
	"..\..\include\util_uri.h"\
	".\os.h"\
	".\readdir.h"\
	

"$(INTDIR)\mod_speling.obj" : $(SOURCE) $(DEP_CPP_MOD_S) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

