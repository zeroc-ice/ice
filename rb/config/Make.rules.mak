# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

OPTIMIZE		= yes

#
# Specify your C++ compiler. Supported values are:
# VC60
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER            = VC60
!endif

#
# Set RUBY_HOME to your Ruby installation directory.
#
!if "$(RUBY_HOME)" == ""
RUBY_HOME		= C:\ruby
!endif

#
# STLPort is required if using MSVC++ 6.0. Change if STLPort
# is located in a different location.
#
!if "$(CPP_COMPILER)" == "VC60" && "$(STLPORT_HOME)" == ""
!if "$(THIRDPARTY_HOME)" != ""
STLPORT_HOME            = $(THIRDPARTY_HOME)
!else
STLPORT_HOME            = C:\Ice-$(VERSION)-ThirdParty-VC60
!endif
!endif


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = rb
ice_require_cpp  = yes
slice_translator = slice2rb.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

libdir			= $(top_srcdir)\ruby
install_rubydir		= $(prefix)\ruby
install_libdir		= $(prefix)\ruby

!if "$(CPP_COMPILER)" != "VC60"
!error Invalid setting for CPP_COMPILER: $(CPP_COMPILER)
!endif

!include $(top_srcdir)\..\cpp\config\Make.rules.msvc

!if "$(CPP_COMPILER)" == "VC60"
libsuff       	= \vc6
!else
libsuff       	= $(x64suffix)
!endif

!if "$(ice_src_dist)" != ""
!if "$(STLPORT_HOME)" != ""
CPPFLAGS        = -I"$(STLPORT_HOME)\include\stlport" $(CPPFLAGS)
LDFLAGS         = /LIBPATH:"$(STLPORT_HOME)\lib$(libsuff)" $(LDFLAGS)
!endif
!else
!if "$(CPP_COMPILER)" == "VC60"
CPPFLAGS        = -I"$(ice_dir)\include\stlport" $(CPPFLAGS)
!endif
!endif

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib slice$(LIBSUFFIX).lib

!if "$(ice_src_dist)" != ""
ICE_CPPFLAGS		= -I"$(ice_cpp_dir)\include"
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
ICE_LDFLAGS		= /LIBPATH:"$(ice_cpp_dir)\lib"
!else
ICE_LDFLAGS		= /LIBPATH:"$(ice_cpp_dir)\lib$(libsuff)"
!endif
!else
ICE_CPPFLAGS		= -I"$(ice_dir)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ice_dir)\lib$(libsuff)"
!endif

slicedir                = $(ice_dir)\slice

RUBY_CPPFLAGS		= -I"$(RUBY_HOME)\lib\ruby\1.8\i386-mswin32"
RUBY_LDFLAGS		= /LIBPATH:"$(RUBY_HOME)\lib"
RUBY_LIBS		= msvcrt-ruby18.lib

ICECPPFLAGS		= -I$(slicedir)
SLICE2RBFLAGS		= $(ICECPPFLAGS)

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2RB                = "$(ice_cpp_dir)\bin\slice2rb.exe"
SLICEPARSERLIB          = "$(ice_cpp_dir)\lib\slice.lib"
!if !exist ($(SLICEPARSERLIB))
SLICEPARSERLIB          = "$(ice_cpp_dir)\lib\sliced.lib"
!endif
!else
SLICE2RB                = "$(ice_cpp_dir)\bin$(x64suffix)\slice2rb.exe"
SLICEPARSERLIB          = "$(ice_cpp_dir)\lib$(x64suffix)\slice.lib"
!if !exist ($(SLICEPARSERLIB))
SLICEPARSERLIB          = "$(ice_cpp_dir)\lib$(x64suffix)\sliced.lib"
!endif
!endif
!else
SLICE2RB                = "$(ice_dir)\bin$(x64suffix)\slice2rb.exe"
SLICEPARSERLIB          = "$(ice_dir)\lib$(x64suffix)\slice.lib"
!if !exist ($(SLICEPARSERLIB))
SLICEPARSERLIB          = "$(ice_dir)\lib$(x64suffix)\sliced.lib"
!endif
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .rb

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

clean::
	del /q $(TARGETS) core *.obj *.bak

all:: $(SRCS) $(TARGETS)

install::
