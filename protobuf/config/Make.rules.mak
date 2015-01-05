# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Protobuf is licensed to you under the terms
# described in the ICE_PROTOBUF_LICENSE file included in this
# distribution.
#
# **********************************************************************

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

#OPTIMIZE		= yes

#
# Specify your C++ compiler. Supported values are:
# VC80, VC90
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER            = VC80
!endif

#
# If Ice is not installed in a standard location, set ICE_HOME to the
# Ice installation directory.
#
#ICE_HOME		= C:\Ice-3.3.0-VC90

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = cpp
ice_require_cpp  = yes
slice_translator = slice2cpp.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

#
# Platform specific definitions
#
!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\cpp\config\Make.rules.msvc
!else
!include $(top_srcdir)\config\Make.rules.msvc
!endif

MT 			= mt.exe

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX	= $(LIBSUFFIX)d
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib

!if "$(ice_src_dist)" != ""
ICE_CPPFLAGS		= -I"$(ice_cpp_dir)\include"
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
ICE_LDFLAGS		= /LIBPATH:"$(ice_cpp_dir)\lib"
!else
ICE_LDFLAGS		= /LIBPATH:"$(ice_cpp_dir)\lib$(x64suffix)"
!endif
!else
ICE_CPPFLAGS		= -I"$(ice_dir)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ice_dir)\lib$(x64suffix)"
!endif

ICECPPFLAGS		= -I"$(slicedir)"
SLICE2CPPFLAGS		= $(ICECPPFLAGS)

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2CPP		= "$(ice_cpp_dir)\bin\slice2cpp.exe"
!else
SLICE2CPP		= "$(ice_cpp_dir)\bin$(x64suffix)\slice2cpp.exe"
!endif
!else
SLICE2CPP		= "$(ice_dir)\bin$(x64suffix)\slice2cpp.exe"
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.ice .cpp .obj

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.ice.cpp:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $<

clean::
	-del /q $(TARGETS)
	-del /q *.obj *.bak *.ilk *.exp *.pdb *.tds *.idb

all:: $(SRCS) $(TARGETS)


