# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

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
# Supported values are: VC100, VC110 or VC120
#
#CPP_COMPILER           = VCxxx 

#
# Is the MFC library available? 
# Set to no if you are using Visual Studio Express
# A few Ice demos use MFC
#
!if "$(HAS_MFC)" == ""
HAS_MFC                 = yes
!endif

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
!else
!error Cannot detect C++ compiler 
!endif

#!message CPP_COMPILER set to $(CPP_COMPILER)
!elseif "$(CPP_COMPILER)" != "VC100" && "$(CPP_COMPILER)" != "VC110" && "$(CPP_COMPILER)" != "VC120"
!error Invalid CPP_COMPILER setting: $(CPP_COMPILER). Must be one of: VC100, VC110 or VC120.
!endif

#
# Common definitions
#
ice_language     = cpp
slice_translator = slice2cpp.exe
ice_require_cpp  = 1

!include $(top_srcdir)\config\Make.common.rules.mak

includedir		= $(ice_dir)\include

SETARGV			= setargv.obj

#
# Compiler specific definitions
#
!include        $(top_srcdir)/config/Make.rules.msvc

!if "$(CPP_COMPILER)" == "VC110"
libsuff                 = \vc110$(x64suffix)
!elseif "$(CPP_COMPILER)" == "VC120"
libsuff                 = \vc120$(x64suffix)
!else
libsuff			= $(x64suffix)
!endif

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX          	= $(LIBSUFFIX)d
RCFLAGS		        = -D_DEBUG
!endif

CPPFLAGS		= $(CPPFLAGS) -I"$(includedir)"
ICECPPFLAGS		= -I"$(slicedir)"
SLICE2CPPFLAGS		= $(ICECPPFLAGS)

LDFLAGS			= $(LDFLAGS) $(PRELIBPATH)"$(ice_dir)\lib$(libsuff)"
LDFLAGS			= $(LDFLAGS) $(LDPLATFORMFLAGS) $(CXXFLAGS)

SLICE2CPP		= $(ice_dir)\bin$(x64suffix)\slice2cpp.exe
!if "$(OPTIMIZE)" != "yes"
SLICEPARSERLIB		= $(SLICE2CPP)
!else
SLICEPARSERLIB		= $(ice_dir)\lib$(x64suffix)\slice$(LIBSUFFIX).lib
!endif
SLICE2XSD		= $(ice_dir)\bin$(x64suffix)\slice2xsd.exe
SLICE2FREEZE		= $(ice_dir)\bin$(x64suffix)\slice2freeze.exe

MT 			= mt.exe

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.ice .cpp .c .obj .res .rc

!if "$(SLICE_OBJS)" != ""

SLICE_OBJS_DEPEND = $(SLICE_OBJS:.obj=.ice.d.mak)

all:: .depend\ice.depend.mak

.depend\ice.depend.mak: Makefile.mak
	@echo Creating Slice dependencies list
	@if not exist ".depend" mkdir .depend
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-list.vbs $(SLICE_OBJS_DEPEND) > .depend\ice.depend.mak

clean::
	-del /q .depend\ice.depend.mak
	-del /q .depend\*.ice.d.mak

!endif

!if exist(.depend\ice.depend.mak)
!include .depend\ice.depend.mak
!endif

!if "$(OBJS)" != ""

OBJS_DEPEND = $(OBJS:.obj=.d.mak)

all:: .depend\depend.mak

.depend\depend.mak: Makefile.mak
	@echo Creating C++ dependencies list
	@if not exist ".depend" mkdir .depend
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-list.vbs $(OBJS_DEPEND) > .depend\depend.mak

clean::
	-del /q .depend\depend.mak
	-del /q .depend\*.d.mak

!endif

!if exist(.depend\depend.mak)
!include .depend\depend.mak
!endif

.cpp.obj:
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) /showIncludes $< | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) /showIncludes $< | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $<

.ice.cpp:
	del /q $(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $(*F).ice
	@"$(SLICE2CPP)" $(SLICE2CPPFLAGS) --depend $(*F).ice > .depend\$(*F).ice.d.mak

.rc.res:
	rc $(RCFLAGS) $<


all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)
!endif

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

clean::
	-del /q *.obj *.bak *.ilk *.exp *.pdb *.tds *.idb

install::
