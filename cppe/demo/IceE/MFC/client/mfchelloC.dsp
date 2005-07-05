# Microsoft Developer Studio Project File - Name="mfchelloC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=mfchelloC - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mfchelloC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mfchelloC.mak" CFG="mfchelloC - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mfchelloC - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "mfchelloC - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "mfchelloC - Win32 Debug Static" (based on "Win32 (x86) Application")
!MESSAGE "mfchelloC - Win32 Release Static" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mfchelloC - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I "../../../../include" /I "../../../../include/stlport" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /incremental:yes /machine:I386 /out:"client.exe" /libpath:"../../../../lib"

!ELSEIF  "$(CFG)" == "mfchelloC - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "../../../../include" /I "../../../../include/stlport" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"client.exe" /pdbtype:sept /libpath:"../../../../lib"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "mfchelloC - Win32 Debug Static"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "mfchelloC___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "mfchelloC___Win32_Debug_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugStatic"
# PROP Intermediate_Dir "DebugStatic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "../../../../include" /I "../../../../include/stlport" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "../../../../include" /I "../../../../include/stlport" /D "_DEBUG" /D "ICEE_STATIC_LIBS" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 IceEd.lib /nologo /subsystem:windows /debug /machine:I386 /out:"client.exe" /pdbtype:sept /libpath:"../../../../lib"
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 ws2_32.lib rpcrt4.lib /nologo /subsystem:windows /debug /machine:I386 /out:"client.exe" /pdbtype:sept /libpath:"../../../../lib"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "mfchelloC - Win32 Release Static"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mfchelloC___Win32_Release_Static"
# PROP BASE Intermediate_Dir "mfchelloC___Win32_Release_Static"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseStatic"
# PROP Intermediate_Dir "ReleaseStatic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I "../../../../include" /I "../../../../include/stlport" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I "../../../../include" /I "../../../../include/stlport" /D "NDEBUG" /D "ICEE_STATIC_LIBS" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 IceE.lib /nologo /subsystem:windows /incremental:yes /machine:I386 /out:"client.exe" /libpath:"../../../../lib"
# ADD LINK32 ws2_32.lib rpcrt4.lib /nologo /subsystem:windows /machine:I386 /out:"client.exe" /libpath:"../../../../lib"
# SUBTRACT LINK32 /incremental:yes

!ENDIF 

# Begin Target

# Name "mfchelloC - Win32 Release"
# Name "mfchelloC - Win32 Debug"
# Name "mfchelloC - Win32 Debug Static"
# Name "mfchelloC - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Hello.cpp
# End Source File
# Begin Source File

SOURCE=.\HelloClient.cpp
# End Source File
# Begin Source File

SOURCE=.\HelloClient.rc
# End Source File
# Begin Source File

SOURCE=.\HelloClientDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Hello.h
# End Source File
# Begin Source File

SOURCE=.\HelloClient.h
# End Source File
# Begin Source File

SOURCE=.\HelloClientDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Hello.ice

!IF  "$(CFG)" == "mfchelloC - Win32 Release"

USERDEP__HELLO="$(ICE_HOME)\bin\slice2cppe.exe"	"$(ICE_HOME)\lib\slice.lib"	
# Begin Custom Build
InputPath=.\Hello.ice

BuildCmds= \
	slice2cppe.exe Hello.ice

"Hello.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Hello.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mfchelloC - Win32 Debug"

USERDEP__HELLO="$(ICE_HOME)\bin\slice2cppe.exe"	"$(ICE_HOME)\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\Hello.ice

BuildCmds= \
	slice2cppe.exe Hello.ice

"Hello.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Hello.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mfchelloC - Win32 Debug Static"

USERDEP__HELLO="$(ICE_HOME)\bin\slice2cppe.exe"	"$(ICE_HOME)\lib\sliced.lib"	
# Begin Custom Build
InputPath=.\Hello.ice

BuildCmds= \
	slice2cppe.exe Hello.ice

"Hello.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Hello.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mfchelloC - Win32 Release Static"

USERDEP__HELLO="$(ICE_HOME)\bin\slice2cppe.exe"	"$(ICE_HOME)\lib\slice.lib"	
# Begin Custom Build
InputPath=.\Hello.ice

BuildCmds= \
	slice2cppe.exe Hello.ice

"Hello.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Hello.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\HelloClient.ico
# End Source File
# Begin Source File

SOURCE=.\res\HelloClient.rc2
# End Source File
# End Group
# End Target
# End Project
