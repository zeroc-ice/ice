# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#
prefix			= C:\IceE-$(VERSION)

#
# Define OPTIMIZE_SIZE as yes if you want to build with minimal size.
# Define OPTIMIZE_SPEED as yes if you want to build with maximum speed.
# These options are mutually exclusive.
# If neither is set, IceE is built with debug information.
#
#OPTIMIZE_SIZE		= yes
#OPTIMIZE_SPEED		= yes

#
# Specify your C++ compiler. Supported values are
# VC80 or VC80_EXPRESS
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER            = VC80
!endif

#
# Define STATICLIBS as yes if you want to build with static libraries.
# Otherwise IceE is built with dynamic libraries. If you want the cpp
# runtime linked statically as well set STATIC_CPP_RUNTIME to yes.
#
#STATICLIBS             = yes
#STATIC_CPP_RUNTIME	= yes

#
# If building for an Windows CE/Mobile embedded device with VS2005 set
# the following two settting to the desired device and OS.
#
# Supported options for EMBEDDED_OS are "WindowsMobile2003",
# "WindowsMobile5.0" and "WindowsMobile6".
#
# Supported options for EMBEDDED_DEVICE are "PocketPC" and "Smartphone"
#
#EMBEDDED_OS            = WindowsMobile2003
#EMBEDDED_DEVICE        = PocketPC

#
# For VC80 and VC80 Express it is necessary to set the location of the
# manifest tool. This must be the 6.x version of mt.exe, not the 5.x
# version!
#
# For VC80 Express mt.exe 6.x is provided by the Windows Platform SDK.
# It is necessary to set the location of the Platform SDK through the
# PDK_HOME environment variable (see INSTALL.WINDOWS for details).
#
!if "$(CPP_COMPILER)" == "VC80"
MT = "$(VS80COMNTOOLS)bin\mt.exe"
!elseif "$(CPP_COMPILER)" == "VC80_EXPRESS"
MT = "$(PDK_HOME)\bin\mt.exe"
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 1.2.0
SOVERSION		= 12
bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib
includedir		= $(top_srcdir)\include
slicedir		= $(top_srcdir)\slice

install_bindir		= $(prefix)\bin
install_libdir	  	= $(prefix)\lib
install_includedir	= $(prefix)\include
install_slicedir	= $(prefix)\slice

INSTALL			= copy
INSTALL_PROGRAM		= $(INSTALL)
INSTALL_LIBRARY		= $(INSTALL)
INSTALL_DATA		= $(INSTALL)

OBJEXT			= .obj

#
# Verify valid embedded settings
#
!if "$(EMBEDDED_DEVICE)" != ""
!if "$(EMBEDDED_DEVICE)" != "PocketPC" && "$(EMBEDDED_DEVICE)" != "Smartphone"
!error Invalid setting for EMBEDDED_DEVICE: "$(EMBEDDED_DEVICE)"
!elseif "$(EMBEDDED_OS)" != "WindowsMobile2003" && "$(EMBEDDED_OS)" != "WindowsMobile5.0" && "$(EMBEDDED_OS)" != "WindowsMobile6"
!error Invalid setting for EMBEDDED_OS: "$(EMBEDDED_OS)"
!endif
!endif


#
# Set executables
#
!if "$(EMBEDDED_DEVICE)" != ""
CXX		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\cl.exe"
CC		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\cl.exe"
LINK 		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\link.exe"
AR		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\lib.exe"
RC		= rc.exe /i "$(VSINSTALLDIR)\VC\ce\atlmfc\include"
!else
CXX		= cl.exe
CC		= cl.exe
LINK 		= link.exe
AR		= lib.exe
RC		= rc.exe
!endif


CPPFLAGS	= -nologo -W3 -GR -EHsc -FD -D_CONSOLE -I$(includedir)

#
# Add options for WinCE support
#
!if "$(EMBEDDED_DEVICE)" != ""

!if "$(EMBEDDED_OS)" == "WindowsMobile2003"

!if "$(EMBEDDED_DEVICE)" == "PocketPC"
SDK_DIR		= $(VSINSTALLDIR)\SmartDevices\SDK\PocketPC2003
!else
SDK_DIR		= $(VSINSTALLDIR)\SmartDevices\SDK\Smartphone2003
!endif

INCLUDE_SUBDIR	= 
LIB_SUBDIR	= \ArmV4
CPPFLAGS	= /D "_WIN32_WCE=0x420" /D "UNDER_CE=0x420" $(CPPFLAGS)
LDFLAGS		= /MACHINE:ARM

!elseif "$(EMBEDDED_OS)" == "WindowsMobile5.0"

!if "$(EMBEDDED_DEVICE)" == "PocketPC"
SDK_DIR		= C:\Program Files\Windows CE Tools\wce500\Windows Mobile 5.0 Pocket PC SDK
!else
SDK_DIR		= C:\Program Files\Windows CE Tools\wce500\Windows Mobile 5.0 Smartphone SDK
!endif

INCLUDE_SUBDIR	= \ArmV4i
LIB_SUBDIR	= \ArmV4i
CPPFLAGS	= /D "_WIN32_WCE=0x501" /D "UNDER_CE=0x501" $(CPPFLAGS)
LDFLAGS		= /MACHINE:THUMB

!else


!if "$(EMBEDDED_DEVICE)" == "PocketPC"
SDK_DIR		= C:\Program Files\Windows Mobile 6 SDK\PocketPC
!else
SDK_DIR		= C:\Program Files\Windows Mobile 6 SDK\Smartphone
!endif

INCLUDE_SUBDIR	= \ArmV4i
LIB_SUBDIR	= \ArmV4i
CPPFLAGS	= /D "_WIN32_WCE=0x502" /D "UNDER_CE=0x502" $(CPPFLAGS)
LDFLAGS		= /MACHINE:THUMB

!endif


CPPFLAGS 	= -QRarch4 -I"$(VSINSTALLDIR)\VC\ce\Include" -I"$(SDK_DIR)\Include$(INCLUDE_SUBDIR)" -fp:fast -TP $(CPPFLAGS) /D "ARM" /D "_ARM_" /D "ARMV4" /D "UNICODE" /D "_UNICODE"

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
CPPFLAGS	= $(CPPFLAGS) -GS-
!endif

LDFLAGS		= /LIBPATH:"$(VSINSTALLDIR)\VC\ce\Lib$(LIB_SUBDIR)" /LIBPATH:"$(SDK_DIR)\Lib$(LIB_SUBDIR)" -nodefaultlib:"kernel32.lib" -nodefaultlib:"oldnames.lib" /STACK:65536,4096 $(LDFLAGS)

!if "$(EMBEDDED_DEVICE)" == "PocketPC"
CPPFLAGS	= $(CPPFLAGS) /D "WIN32_PLATFORM_PSPC"
!elseif "$(EMBEDDED_DEVICE)" == "Smartphone"
CPPFLAGS	= $(CPPFLAGS) /D "WIN32_PLATFORM_WFSP"
!endif

!endif

#
# Add options for static library support
#
!if "$(STATICLIBS)" == "yes"
CPPFLAGS	= $(CPPFLAGS) -DICEE_STATIC_LIBS
!endif

#
# Add release vs debug options
#
!if "$(OPTIMIZE_SPEED)" == "yes" || "$(OPTIMIZE_SIZE)" == "yes"

CPPFLAGS	= $(CPPFLAGS) -DNDEBUG -GL

!if "$(STATICLIBS)" == "yes" && "$(STATIC_CPP_RUNTIME)" == "yes"
CPPFLAGS        = $(CPPFLAGS) -MT
!else
CPPFLAGS        = $(CPPFLAGS) -MD
!endif

!if "$(OPTIMIZE_SPEED)" == "yes"
CPPFLAGS	= $(CPPFLAGS) -O2
!else
CPPFLAGS        = $(CPPFLAGS) -O1
!endif
 
!else # Debug

CPPFLAGS	= $(CPPFLAGS) -Zi -Gm -Od -D_DEBUG

!if "$(STATICLIBS)" == "yes" && "$(STATIC_CPP_RUNTIME)" == "yes"
CPPFLAGS        = $(CPPFLAGS) -MTd
!else
CPPFLAGS        = $(CPPFLAGS) -MDd
!endif

!if "$(EMBEDDED_DEVICE)" == ""
CPPFLAGS        = $(CPPFLAGS) -RTC1
!endif

!endif

#
# Linker flags
#
LDFLAGS         = $(LDFLAGS) /LIBPATH:"$(libdir)" /nologo
!if "$(EMBEDDED_DEVICE)" != ""
LDFLAGS		= $(LDFLAGS) /subsystem:windowsce -manifest:no
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
LDFLAGS		= $(LDFLAGS) /debug /fixed:no /incremental:yes
!else
LDFLAGS         = $(LDFLAGS) /OPT:REF /incremental:no /OPT:NOWIN98 /LTCG
!endif

ARFLAGS		= /nologo
!if "$(OPTIMIZE_SPEED)" == "yes" || "$(OPTIMIZE_SIZE)" == "yes"
ARFLAGS		= $(ARFLAGS) /LTCG
!endif

#
# MFC specific flags
#

!if "$(EMBEDDED_DEVICE)" != ""

MFC_CPPFLAGS	= -I"$(VSINSTALLDIR)\VC\ce\atlmfc\include" 
MFC_LDFLAGS	= /LIBPATH:"$(VSINSTALLDIR)\VC\ce\atlmfc\lib$(LIB_SUBDIR)"

!else

MFC_LDFLAGS     = /subsystem:windows

!endif

#
# RC specific flags
#
!if "$(EMBEDDED_OS)" == "WindowsMobile2003"
RC		= $(RC) /d "UNDER_CE=0x420" /d "_WIN32_WCE=0x420"
!elseif "$(EMBEDDED_OS)" == "WindowsMobile5.0"
RC		= $(RC) /d "UNDER_CE=0x501" /d "_WIN32_WCE=0x501"
!elseif "$(EMBEDDED_OS)" == "WindowsMobile6"
RC		= $(RC) /d "UNDER_CE=0x502" /d "_WIN32_WCE=0x502"
!endif

#
# Set up libraries
#
!if "$(STATICLIBS)" == "yes"
LIBSUFFIX	= _static$(LIBSUFFIX)
!endif
!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
LIBSUFFIX	= $(LIBSUFFIX)d
!endif


!if "$(EMBEDDED_DEVICE)" != ""

BASELIBS	= coredll.lib ws2.lib
!if "$(EMBEDDED_OS)" == "WindowsMobile2003"
BASELIBS        = $(BASELIBS) ccrtrtti.lib
!else
BASELIBS        = $(BASELIBS) corelibc.lib 
!endif

!else

BASELIBS        = rpcrt4.lib advapi32.lib ws2_32.lib
!if "$(STATICLIBS)" == "yes" && "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
BASELIBS	= $(BASELIBS)
!endif

!endif

LIBS		= icee$(LIBSUFFIX).lib
MINLIBS		= iceec$(LIBSUFFIX).lib

!if "$(STATICLIBS)" == "yes" || "$(EMBEDDED_DEVICE)" != ""
LIBS		= $(LIBS) $(BASELIBS)
MINLIBS		= $(MINLIBS) $(BASELIBS)
!endif

MFC_LIBS	= $(LIBS)
MFC_MINLIBS	= $(MINLIBS)
!if "$(EMBEDDED_DEVICE)" == "" && "$(STATICLIBS)" == "yes" && "$(STATIC_CPP_RUNTIME)" == "yes"
!if  "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
MFC_LIBS        = nafxcwd.lib $(LIBS)
MFC_MINLIBS	= nafxcwd.lib $(MINLIBS)
!else
MFC_LIBS        = nafxcw.lib $(LIBS)
MFC_MINLIBS	= nafxcw.lib $(MINLIBS)
!endif
!endif

TESTLIBS	= testcommon$(LIBSUFFIX).lib $(LIBS)

SLICE2CPPEFLAGS		= -I$(slicedir)
SLICE2CPP		= slice2cppe.exe

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.ice .cpp .c .obj

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) $<


{$(SDIR)\}.ice{$(HDIR)}.h:
	del /q $(HDIR)\$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPEFLAGS) $<
	move $(*F).h $(HDIR)

.ice.cpp:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPEFLAGS) $(*F).ice

all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)

!endif

clean::
	-del /q *.obj *.bak *.ilk *.exp *.pdb

install::
