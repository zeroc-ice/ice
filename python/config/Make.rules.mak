# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

!if "$(PREFIX)" == ""
prefix			= C:\Ice-$(VERSION)
!else
prefix			= $(PREFIX)
!endif

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

OPTIMIZE		= yes

#
# Specify your C++ compiler, or leave unset for auto-detection. The
# only value currently supported to build IcePy is VC100.
#
#CPP_COMPILER 		= VCxxx

#
# Set PYTHON_HOME to your Python installation directory.
#
!if "$(PYTHON_HOME)" == ""
PYTHON_HOME		= C:\Python34
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = python
ice_require_cpp  = yes

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

libdir			= $(top_srcdir)\python
install_pythondir	= $(prefix)\python$(x64suffix)
install_libdir		= $(prefix)\python$(x64suffix)

!include $(top_srcdir)\..\cpp\config\Make.rules.msvc

libsuff			= $(x64suffix)

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
PYLIBSUFFIX     = _$(LIBSUFFIX)
RCFLAGS		= -D_DEBUG
!endif

#
# Import libraries are located automatically
#
ICE_LIBS		=

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

PYTHON_CPPFLAGS		= -I"$(PYTHON_HOME)\include"
PYTHON_LDFLAGS		= /LIBPATH:"$(PYTHON_HOME)\libs"

ICECPPFLAGS		= -I"$(slicedir)"
SLICE2PYFLAGS		= $(ICECPPFLAGS)

!if "$(ice_src_dist)" != ""
SLICE2PY                = "$(PYTHON_HOME)\python" $(top_srcdir)\config\s2py.py
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2PY                = "$(PYTHON_HOME)\Scripts\slice2py.exe"
SLICEPARSERLIB          = "$(PYTHON_HOME)\Scripts\slice2py.exe"
!endif

MT			= mt.exe

EVERYTHING		= all clean install depend
EVERYTHING_EXCEPT_INSTALL = all clean depend

.SUFFIXES:
.SUFFIXES:		.cpp .obj .py .res .rc .d .ice

DEPEND_DIR = .depend.mak

depend::

!if exist(.depend.mak)
!include .depend.mak
!endif

!if "$(OBJS)" != ""
depend::
	@del /q .depend.mak

OBJS_DEPEND = $(OBJS:.obj=.d)
OBJS_DEPEND = $(OBJS_DEPEND:.\=.depend.mak\)

depend:: $(OBJS_DEPEND)

!endif

.cpp{$(DEPEND_DIR)}.d:
	@echo Generating dependencies for $<
	@$(CXX) /E $(CPPFLAGS) $(CXXFLAGS) /showIncludes $< 1>$(*F).i 2>$(*F).d && \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $(*F).cpp $(top_srcdir)
	@del /q $(*F).d $(*F).i

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.rc.res:
	rc $(RCFLAGS) $<

clean::
	del /q $(TARGETS) *.obj *.pyc *.bak

all:: $(SRCS) $(TARGETS)

install::
