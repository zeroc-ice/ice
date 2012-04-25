# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
STLPORT_HOME            = C:\Program Files\ZeroC\Ice-$(VERSION)-ThirdParty
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
RCFLAGS		= -D_DEBUG
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

#
# Ruby 1.8
#
RUBY_CPPFLAGS		= -I"$(RUBY_HOME)\lib\ruby\1.8\i386-mswin32"
RUBY_LDFLAGS		= /LIBPATH:"$(RUBY_HOME)\lib"
RUBY_LIBS		= msvcrt-ruby18.lib

#
# Ruby 1.9
#
# Without HAVE_VSNPRINTF the Ruby header files declare this function, which causes
# a compilation error in Util.cpp.
#
#RUBY_CPPFLAGS		= -I"$(RUBY_HOME)\include\ruby-1.9.1" -I"$(RUBY_HOME)\include\ruby-1.9.1\i386-mswin32" -DWIN32_LEAN_AND_MEAN -DHAVE_VSNPRINTF
#RUBY_LDFLAGS		= /LIBPATH:"$(RUBY_HOME)\lib"
#RUBY_LIBS		= msvcrt-ruby191.lib

ICECPPFLAGS		= -I"$(slicedir)"
SLICE2RBFLAGS		= $(ICECPPFLAGS)

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2RB                = $(ice_cpp_dir)\bin\slice2rb.exe
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2RB                = $(ice_cpp_dir)\bin$(x64suffix)\slice2rb.exe
SLICEPARSERLIB          = $(ice_cpp_dir)\lib$(libsuff)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib$(libsuff)\sliced.lib
!endif
!endif
!else
SLICE2RB                = $(ice_dir)\bin$(x64suffix)\slice2rb.exe
SLICEPARSERLIB          = $(ice_dir)\lib$(libsuff)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_dir)\lib$(libsuff)\sliced.lib
!endif
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .rb .res .rc

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.rc.res:
	rc $(RCFLAGS) $<

clean::
	del /q $(TARGETS) core *.obj *.bak

all:: $(SRCS) $(TARGETS)

install::
