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
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

OPTIMIZE		= yes

#
# Specify your C++ compiler, or leave unset for auto-detection.
# The only value currently supported is VC100.
#
# CPP_COMPILER = VCxxx

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
# Check CPP_COMPILER
#
!if "$(CPP_COMPILER)" == ""

!if "$(VISUALSTUDIOVERSION)" == "11.0"
!error Detected VC110
!elseif ([cl 2>&1 | findstr "Version\ 16" > nul] == 0)
CPP_COMPILER            = VC100
!elseif ([cl 2>&1 | findstr "Version\ 15" > nul] == 0)
!error Detected VC90
!elseif ([cl 2>&1 | findstr "Version\ 17" > nul] == 0)
!error Detected VC110
!elseif ([cl 2>&1 | findstr "Version\ 18" > nul] == 0)
!error Detected VC120
!else
!error Cannot detect C++ compiler 
!endif

!elseif "$(CPP_COMPILER)" != "VC100"
!error Invalid CPP_COMPILER setting: $(CPP_COMPILER). Must be set to VC100.
!endif


#
# Common definitions
#
ice_language     = py
ice_require_cpp  = yes
slice_translator = slice2py.exe

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
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2PY                = $(ice_cpp_dir)\bin\slice2py.exe
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2PY                = $(ice_cpp_dir)\bin$(x64suffix)\slice2py.exe
SLICEPARSERLIB          = $(ice_cpp_dir)\lib$(x64suffix)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib$(x64suffix)\sliced.lib
!endif
!endif
!else
SLICE2PY                = $(ice_dir)\bin\slice2py.exe
SLICEPARSERLIB          = $(ice_dir)\lib\slice.lib
!endif

MT			= mt.exe

EVERYTHING		= all clean install depend

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
