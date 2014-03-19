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
prefix			= C:\IceJS-$(ICEJS_VERSION)

#
# Define to yes for an optimized build.
#
#OPTIMIZE 		= yes

#
# Google Closure Compiler
#
CLOSURE_PATH		= C:\closure

#
# Closure Flags
#
CLOSUREFLAGS 		= --language_in ECMASCRIPT5

#
# jsHint location
#
JSHINT_PATH 		= $(NODE_PATH)\jshint

#
# Define to the location of gnu gzip if you want to generate
# gzip version of JavaScript libraries.
#
#GZIP_PATH		= "C:\Program Files (x86)\GnuWin32\bin\gzip.exe"

#
# jsHint flags
#
LINTFLAGS 		= --verbose

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language = js
slice_translator = slice2js.exe

bindir = $(top_srcdir)\bin
libdir = $(top_srcdir)\lib

install_libdir 	  = $(prefix)\lib
install_moduledir = $(prefix)\node_modules

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2JS                = $(ice_cpp_dir)\bin\slice2js.exe
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2JS                = $(ice_cpp_dir)\bin$(x64suffix)\slice2js.exe
SLICEPARSERLIB          = $(SLICE2JS)
!endif
!else
SLICE2JS                = $(ice_js_dir)\bin$(x64suffix)\slice2js.exe
SLICEPARSERLIB          = $(SLICE2JS)
!endif

!if "$(OPTIMIZE)" == "yes"
jslibsuffix     = .min.js
!else
jslibsuffix     = .js
!endif

!if "$(LIBNAME)" != ""
TARGETS = $(TARGETS) $(libdir)\$(LIBNAME)$(jslibsuffix)
install:: all
        copy $(libdir)\$(LIBNAME)$(jslibsuffix) $(install_libdir)
!if "$(GZIP_PATH)" != ""
TARGETS = $(TARGETS) $(libdir)\$(LIBNAME)$(jslibsuffix).gz
install:: all
        copy $(libdir)\$(LIBNAME)$(jslibsuffix).gz $(install_libdir)
!endif
!endif

EVERYTHING		= all clean install lint

.SUFFIXES:
.SUFFIXES:		.js .ice

.ice.js:
        "$(SLICE2JS)" $(SLICE2JSFLAGS) $<

{$(SDIR)}.ice.js:
        "$(SLICE2JS)" $(SLICE2JSFLAGS) $<

all:: $(TARGETS)

!if "$(TARGETS)" != ""
clean::
	del /q $(TARGETS)
!endif

!if "$(GEN_SRCS)" != ""
clean::
	del /q $(GEN_SRCS)
!endif

index.html: $(GEN_SRCS) $(top_srcdir)\test\Common\index.html
	copy $(top_srcdir)\test\Common\index.html .

$(libdir)/$(LIBNAME).js: $(SRCS)
	@del /q $(libdir)\$(LIBNAME).js
	node $(top_srcdir)\config\makebundle.js $(SRCS) > $(libdir)\$(LIBNAME).js

!if "$(OPTIMIZE)" == "yes"
$(libdir)/$(LIBNAME).min.js: $(libdir)/$(LIBNAME).js
	@del /q $(libdir)\$(LIBNAME).min.js
	node $(top_srcdir)\config\makebundle.js $(SRCS) > $(libdir)\$(LIBNAME).tmp.js
	java -jar $(CLOSURE_PATH)\compiler.jar $(CLOSUREFLAGS) --js $(libdir)\$(LIBNAME).js --js_output_file $(libdir)\$(LIBNAME).min.js
	del /q $(libdir)\$(LIBNAME).tmp.js
!endif

!if "$(GZIP_PATH)" != ""
$(libdir)/$(LIBNAME)$(jslibsuffix).gz: $(libdir)/$(LIBNAME)$(jslibsuffix)
        @del /q $(libdir)\$(LIBNAME)$(jslibsuffix).gz
        "$(GZIP_PATH)" -c9 $(libdir)\$(LIBNAME)$(jslibsuffix) > $(libdir)\$(LIBNAME)$(jslibsuffix).gz
!endif

!if "$(INSTALL_SRCS)" != ""
lint: $(INSTALL_SRCS)
	node "$(JSHINT_PATH)\bin\jshint" $(LINTFLAGS) $(INSTALL_SRCS)
!else
lint::
!endif

install::
