# Microsoft Developer Studio Project File - Name="Thread_PerConnectionTAOC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Thread_PerConnectionTAOC - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Thread_Per_ConnectionTAOC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Thread_Per_ConnectionTAOC.mak" CFG="Thread_PerConnectionTAOC - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Thread_PerConnectionTAOC - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Thread_PerConnectionTAOC - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Thread_PerConnectionTAOC - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "Debug\Thread_PerConnectionTAOC"
# PROP Target_Dir ""
MTL=midl.exe
# ADD MTL /D "_DEBUG" /nologo /mktyplib203 /win32
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Gy /I "$(TAO_ROOT)\.." /I "$(TAO_ROOT)" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 advapi32.lib user32.lib TAO_Strategiesd.lib TAO_PortableServerd.lib TAOd.lib ACEd.lib IceUtil.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:".\client.exe" /libpath:"$(TAO_ROOT)\..\ace" /libpath:"$(TAO_ROOT)\tao" /libpath:"$(TAO_ROOT)\tao\Strategies" /libpath:"$(TAO_ROOT)\tao\PortableServer" /libpath:"$(ICE_HOME)\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Thread_PerConnectionTAOC - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD MTL /D "NDEBUG" /nologo /mktyplib203 /win32
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(TAO_ROOT)\.." /I "$(TAO_ROOT)" /I "$(ICE_HOME)\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 advapi32.lib user32.lib TAO_Strategies.lib TAO_PortableServer.lib TAO.lib TAO_ValueType.lib TAO_Messaging.lib ACE.lib IceUtil.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"client.exe" /libpath:"$(TAO_ROOT)\..\ace" /libpath:"$(TAO_ROOT)\tao" /libpath:"$(TAO_ROOT)\tao\Strategies" /libpath:"$(TAO_ROOT)\tao\Messaging" /libpath:"$(TAO_ROOT)\tao\ValueType" /libpath:"$(TAO_ROOT)\tao\PortableServer" /libpath:"$(ICE_HOME)\lib"

!ENDIF 

# Begin Target

# Name "Thread_PerConnectionTAOC - Win32 Debug"
# Name "Thread_PerConnectionTAOC - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;cxx;c"
# Begin Source File

SOURCE="client.cpp"
# End Source File
# Begin Source File

SOURCE=.\Roundtrip_Handler.cpp
# End Source File
# Begin Source File

SOURCE="TestC.cpp"
# End Source File
# Begin Source File

SOURCE=.\TestS.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hh"
# Begin Source File

SOURCE=.\Roundtrip_Handler.h
# End Source File
# Begin Source File

SOURCE="TestC.h"
# End Source File
# Begin Source File

SOURCE="TestS.h"
# End Source File
# Begin Source File

SOURCE="TestS_T.h"
# End Source File
# End Group
# Begin Group "Inline Files"

# PROP Default_Filter "i;inl"
# Begin Source File

SOURCE="TestC.inl"
# End Source File
# Begin Source File

SOURCE="TestS.inl"
# End Source File
# Begin Source File

SOURCE="TestS_T.inl"
# End Source File
# End Group
# Begin Group "Template Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="TestS_T.cpp"
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE="README"
# End Source File
# End Group
# Begin Group "Idl Files"

# PROP Default_Filter "idl"
# Begin Source File

SOURCE="Test.idl"

!IF  "$(CFG)" == "Thread_PerConnectionTAOC - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__TEST_="$(TAO_ROOT)\..\bin\Release\tao_idl.exe"	
# Begin Custom Build - Invoking $(TAO_ROOT)\..\bin\Release\tao_idl on $(InputPath)
InputPath="Test.idl"

BuildCmds= \
	PATH=%PATH%;$(TAO_ROOT)\..\lib \
	$(TAO_ROOT)\..\bin\Release\tao_idl -Ge 1 -GC -Wb,pre_include=ace\pre.h -Wb,post_include=ace\post.h -I$(TAO_ROOT) $(InputPath) \
	

"TestC.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS_T.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestC.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS_T.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestC.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS_T.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Thread_PerConnectionTAOC - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__TEST_="$(TAO_ROOT)\..\bin\Release\tao_idl.exe"	
# Begin Custom Build - Invoking $(TAO_ROOT)\..\bin\Release\tao_idl on $(InputPath)
InputPath="Test.idl"

BuildCmds= \
	PATH=%PATH%;$(TAO_ROOT)\..\lib \
	$(TAO_ROOT)\..\bin\Release\tao_idl -Ge 1 -GC -Wb,pre_include=ace\pre.h -Wb,post_include=ace\post.h -I$(TAO_ROOT) $(InputPath) \
	

"TestC.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS_T.inl" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestC.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS_T.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestC.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"TestS_T.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
