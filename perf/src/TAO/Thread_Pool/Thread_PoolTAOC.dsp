# Microsoft Developer Studio Project File - Name="Thread_PoolTAOC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Thread_PoolTAOC - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE run the tool that generated this project file and specify the
!MESSAGE nmake output type.  You can then use the following command:
!MESSAGE
!MESSAGE NMAKE.
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE CFG="Thread_PoolTAOC - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Thread_PoolTAOC - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Thread_PoolTAOC - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Thread_PoolTAOC - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "Debug\Thread_PoolTAOC"
# PROP Target_Dir ""
# ADD CPP /nologo /Ob0 /W3 /Gm /GX /Zi /MDd /GR /Gy /Fd"Debug\Thread_PoolTAOC/" /I "$(TAO_ROOT)\.." /I "$(TAO_ROOT)" /D _DEBUG /D WIN32 /D _CONSOLE /FD /c
# SUBTRACT CPP /YX

# ADD MTL /D "_DEBUG" /nologo /mktyplib203 /win32
# ADD RSC /l 0x409 /d _DEBUG /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)"
BSC32=bscmake.exe
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD LINK32 advapi32.lib user32.lib /INCREMENTAL:NO TAO_Strategiesd.lib TAOd.lib ACEd.lib /libpath:"$(TAO_ROOT)\..\lib" /libpath:"$(TAO_ROOT)\..\lib" /nologo /subsystem:console /pdb:".\client.pdb" /debug /machine:I386 /out:".\client.exe"

!ELSEIF  "$(CFG)" == "Thread_PoolTAOC - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Thread_PoolTAOC"
# PROP Target_Dir ""
# ADD CPP /nologo /O2 /W3 /GX /MD /GR /I "$(TAO_ROOT)\.." /I "$(TAO_ROOT)" /D NDEBUG /D WIN32 /D _CONSOLE /FD /c
# SUBTRACT CPP /YX

# ADD MTL /D "NDEBUG" /nologo /mktyplib203 /win32
# ADD RSC /l 0x409 /d NDEBUG /i "$(TAO_ROOT)\.." /i "$(TAO_ROOT)"
BSC32=bscmake.exe
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD LINK32 advapi32.lib user32.lib /INCREMENTAL:NO TAO_Strategies.lib TAO.lib ACE.lib /libpath:"$(TAO_ROOT)\..\lib" /libpath:"$(TAO_ROOT)\..\lib" /nologo /subsystem:console /pdb:none  /machine:I386 /out:"Release\client.exe"

!ENDIF

# Begin Target

# Name "Thread_PoolTAOC - Win32 Debug"
# Name "Thread_PoolTAOC - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;cxx;c"
# Begin Source File

SOURCE="client.cpp"
# End Source File
# Begin Source File

SOURCE="Client_Task.cpp"
# End Source File
# Begin Source File

SOURCE="TestC.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hh"
# Begin Source File

SOURCE="Client_Task.h"
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

!IF  "$(CFG)" == "Thread_PoolTAOC - Win32 Debug"

USERDEP__Test=$(TAO_ROOT)\..\bin\tao_idl.exe
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Invoking $(TAO_ROOT)\..\bin\tao_idl on $(InputPath)
InputPath="Test.idl"

BuildCmds= \
	PATH=%PATH%;$(TAO_ROOT)\..\lib \
	$(TAO_ROOT)\..\bin\tao_idl -Ge 1 -Wb,pre_include=ace\pre.h -Wb,post_include=ace\post.h -I$(TAO_ROOT) $(InputPath) \

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

!ELSEIF  "$(CFG)" == "Thread_PoolTAOC - Win32 Release"

USERDEP__Test=$(TAO_ROOT)\..\bin\tao_idl.exe
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Invoking $(TAO_ROOT)\..\bin\tao_idl on $(InputPath)
InputPath="Test.idl"

BuildCmds= \
	PATH=%PATH%;$(TAO_ROOT)\..\lib \
	$(TAO_ROOT)\..\bin\tao_idl -Ge 1 -Wb,pre_include=ace\pre.h -Wb,post_include=ace\post.h -I$(TAO_ROOT) $(InputPath) \

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
