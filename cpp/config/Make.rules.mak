# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#
prefix			= C:\Ice-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with
# optimization. Otherwise Ice is build with debug information.
#
#OPTIMIZE		= yes

#
# Define if you want pdb files to be generated for optimized/release
# builds
#
#RELEASEPDBS            = yes

#
# Specify your C++ compiler, or leave unset for auto-detection.
# Supported values are: VC90, VC100, VC110, VC120
#
# CPP_COMPILER = VCxxx

#
# Define if you want to build for WinRT
#
#WINRT		        = yes

#
# Is the MFC library available? 
# Set to no if you are using Visual Studio Express
# A few Ice demos use MFC
#
!if "$(HAS_MFC)" == ""
HAS_MFC                 = yes
!endif

#
# If third party libraries are not installed in the default location
# or THIRDPARTY_HOME is not set in your environment variables then
# change the following setting to reflect the installation location.
#
!if "$(THIRDPARTY_HOME)" == ""
!if "$(PROCESSOR_ARCHITECTURE)" == "AMD64"
THIRDPARTY_HOME = $(PROGRAMFILES) (x86)\ZeroC\Ice-$(VERSION)-ThirdParty
!else
THIRDPARTY_HOME = $(PROGRAMFILES)\ZeroC\Ice-$(VERSION)-ThirdParty 
!endif
!endif

#
# Define if you want the Ice DLLs to have compiler specific names.
# Will be set to yes by default when CPP_COMPILER=VC90, and unset 
# otherwise 
#
#UNIQUE_DLL_NAMES	= yes


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Check CPP_COMPILER
#
!if "$(CPP_COMPILER)" == ""

!if "$(VISUALSTUDIOVERSION)" == "11.0"
CPP_COMPILER            = VC110
!elseif ([cl 2>&1 | findstr "Version\ 16" > nul] == 0)
CPP_COMPILER            = VC100
!elseif ([cl 2>&1 | findstr "Version\ 17" > nul] == 0)
CPP_COMPILER            = VC110
!elseif ([cl 2>&1 | findstr "Version\ 18" > nul] == 0)
CPP_COMPILER            = VC120
!elseif ([cl 2>&1 | findstr "Version\ 15" > nul] == 0)
CPP_COMPILER            = VC90
!else
!error Cannot detect C++ compiler 
!endif

#!message CPP_COMPILER set to $(CPP_COMPILER)
!elseif "$(CPP_COMPILER)" != "VC90" && "$(CPP_COMPILER)" != "VC100" && "$(CPP_COMPILER)" != "VC110" && "$(CPP_COMPILER)" != "VC120"
!error Invalid CPP_COMPILER setting: $(CPP_COMPILER). Must be one of: VC90, VC100, VC110 or VC120.
!endif

#
# With VC90, we want unique dll names by default
#
!if "$(CPP_COMPILER)" == "VC90" && "$(UNIQUE_DLL_NAMES)" == ""
UNIQUE_DLL_NAMES	= yes
!endif

#
# Common definitions
#
ice_language     = cpp
!if "$(USE_BIN_DIST)" == "yes" || !exist ($(top_srcdir)\..\cpp)
slice_translator = slice2cpp.exe
ice_require_cpp  = 1
!endif

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib
headerdir		= $(top_srcdir)\include

!if "$(ice_src_dist)" != ""
includedir		= $(top_srcdir)\include
!else
includedir		= $(ice_dir)\include
!endif

install_bindir		= $(prefix)\bin$(x64suffix)
install_libdir	  	= $(prefix)\lib$(x64suffix)
install_includedir	= $(prefix)\include
install_docdir		= $(prefix)\doc
install_configdir	= $(prefix)\config

SETARGV			= setargv.obj

#
# Compiler specific definitions
#
!include        $(top_srcdir)/config/Make.rules.msvc

!if "$(WINRT)" == "yes" && "$(CPP_COMPILER)" != "VC110" && "$(CPP_COMPILER)" != "VC120"
!error CPP_COMPILER: $(CPP_COMPILER) not supported to build Ice for WinRT
!endif

!if "$(CPP_COMPILER)" == "VC90"
libsuff			= \vc90$(x64suffix)
!elseif "$(CPP_COMPILER)" == "VC110"
libsuff                 = \vc110$(x64suffix)
!elseif "$(CPP_COMPILER)" == "VC120"
libsuff                 = \vc120$(x64suffix)
!else
libsuff			= $(x64suffix)
!endif

!if "$(ice_src_dist)" != ""
!if "$(THIRDPARTY_HOME)" != ""
CPPFLAGS        = -I"$(THIRDPARTY_HOME)\include" $(CPPFLAGS)
LDFLAGS         = $(PRELIBPATH)"$(THIRDPARTY_HOME)\lib$(libsuff)" $(LDFLAGS)
!endif
!else
!endif

!if "$(UNIQUE_DLL_NAMES)" == "yes"
!if "$(CPP_COMPILER)" == "VC90"
COMPSUFFIX	= _vc90
!elseif "$(CPP_COMPILER)" == "VC100"
COMPSUFFIX	= _vc100
!elseif "$(CPP_COMPILER)" == "VC110"
COMPSUFFIX  = _vc110
!elseif "$(CPP_COMPILER)" == "VC120"
COMPSUFFIX  = _vc120
!endif
!endif

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX	= d
RCFLAGS		= -D_DEBUG
!endif

OPENSSL_LIBS            = ssleay32.lib libeay32.lib
EXPAT_LIBS              = libexpat.lib

#
# QT is used only for the deprecated IceGrid and IceStorm SQL plugins
#
QT_LIBS			= QtSql$(LIBSUFFIX)4.lib QtCore$(LIBSUFFIX)4.lib

CPPFLAGS		= $(CPPFLAGS) -I"$(includedir)"
ICECPPFLAGS		= -I"$(slicedir)"
SLICE2CPPFLAGS		= $(ICECPPFLAGS) $(SLICE2CPPFLAGS)

!if "$(WINRT)" != "yes"
!if "$(ice_src_dist)" != ""
LDFLAGS			= $(LDFLAGS) $(PRELIBPATH)"$(libdir)"
!else
LDFLAGS			= $(LDFLAGS) $(PRELIBPATH)"$(ice_dir)\lib$(libsuff)"
!endif
!else
LDFLAGS			= $(LDFLAGS) $(PRELIBPATH)"$(SDK_LIBRARY_PATH)"
!endif

LDFLAGS			= $(LDFLAGS) $(LDPLATFORMFLAGS) $(CXXFLAGS)

!if "$(ice_src_dist)" != ""
SLICEPARSERLIB		= $(libdir)\slice$(LIBSUFFIX).lib
SLICE2CPP		= $(bindir)\slice2cpp.exe
SLICE2FREEZE		= $(bindir)\slice2freeze.exe
!else
SLICE2CPP		= $(ice_dir)\bin$(x64suffix)\slice2cpp.exe
SLICEPARSERLIB		= $(SLICE2CPP)
SLICE2FREEZE		= $(ice_dir)\bin$(x64suffix)\slice2freeze.exe
!endif

#
# In WinRT tests we don't want a dependency on SLICEPARSELIB, as we can build all 
# test configurations using the same slice2cpp and slice.lib.
#
!if "$(WINRT)" == "yes"
SLICEPARSERLIB	= $(SLICE2CPP)
!endif

MT 			= mt.exe

EVERYTHING		= all clean install depend

.SUFFIXES:
.SUFFIXES:		.ice .cpp .c .obj .res .rc .h .depend


.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) $<

{$(SDIR)\}.ice{$(HDIR)}.h:
	del /q $(HDIR)\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $<
	move $(*F).h $(HDIR)

.ice.cpp:
	del /q $(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $(*F).ice

!if "$(WINRT)" == "yes"

{..}.cpp{$(ARCH)\$(CONFIG)\}.obj::
	@if not exist "$(ARCH)\$(CONFIG)" mkdir $(ARCH)\$(CONFIG)
	$(CXX) /c /Fo$(ARCH)\$(CONFIG)\ $(CPPFLAGS) $(CXXFLAGS) $<

{$(SDIR)\}.ice{..}.cpp:
	@echo f
	del /q $(HDIR)\$(*F).h ..\$(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $<
	move $(*F).h $(HDIR)
	move $(*F).cpp ..

!if "$(SRCS)" != ""
SRCS_DEPEND 	= $(SRCS:.cpp=.depend)
!endif

!if "$(INCLUDE_DIR)" != ""
.h{$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\}.h:
	copy $(*F).h $(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)

$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR):
	mkdir $(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)

all:: $(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)
!endif

!endif

.rc.res:
	rc $(RCFLAGS) $<


all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)

!endif

!if "$(WINRT)" != "yes"

# Suffix set, we're using a debug build.
!if "$(LIBSUFFIX)" != ""

!if "$(LIBNAME)" != ""
clean::
	-del /q $(LIBNAME:d.lib=.lib)
	-del /q $(LIBNAME)
!endif
!if "$(DLLNAME)" != ""
clean::
	-del /q $(DLLNAME:d.dll=.*)
	-del /q $(DLLNAME:.dll=.*)
!endif

!else

!if "$(LIBNAME)" != ""
clean::
	-del /q $(LIBNAME:.lib=d.lib)
	-del /q $(LIBNAME)
!endif
!if "$(DLLNAME)" != ""
clean::
	-del /q $(DLLNAME:.dll=d.*)
	-del /q $(DLLNAME:.dll=.*)
!endif

!endif

!endif 

clean::
	-del /q *.obj *.bak *.ilk *.exp *.pdb *.tds *.idb

install::

depend::
