# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Define to yes for an optimized build.
#
OPTIMIZE ?= no

#
# Google Closure Compiler
#
CLOSURE_PATH=/opt/closure

#
# Closure Flags
#
CLOSUREFLAGS = --language_in ECMASCRIPT5

#
# jslint flags
#
LINTFLAGS = --verbose

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = js
slice_translator = slice2js
ice_require_cpp  = 1

include $(top_srcdir)/config/Make.common.rules

includedir		= $(ice_dir)/include
libdir			= $(ice_js_dir)/lib

#
# Platform specific definitions
#
include	 $(top_srcdir)/config/Make.rules.$(UNAME)

ICECPPFLAGS		= -I$(slicedir)

SLICE2CPPFLAGS		= $(ICECPPFLAGS)

ifeq ($(ice_dir), /usr) 
    LDFLAGS	= $(LDPLATFORMFLAGS) $(CXXFLAGS)
else
    CPPFLAGS	+= -I$(includedir)
    LDFLAGS	= $(LDPLATFORMFLAGS) $(CXXFLAGS) -L$(ice_dir)/$(libsubdir)$(cpp11suffix)
endif


SLICE2JS	= $(ice_js_dir)/$(binsubdir)$(cpp11suffix)/slice2js

EVERYTHING		= all clean
EVERYTHING_EXCEPT_ALL   = clean

.SUFFIXES:
.SUFFIXES:		.js

%.js: %.ice $(SLICE2JS) $(SLICEPARSERLIB)
	rm -f $(*F).js
	$(SLICE2JS) $(SLICE2JSFLAGS) $<

all:: $(SRCS) $(TARGETS)

clean::
	-rm -f $(TARGETS)
	-rm -f core *.o *.bak

ifneq ($(SLICE_SRCS),)
clean::
	rm -f $(addsuffix .cpp, $(basename $(notdir $(SLICE_SRCS))))
	rm -f $(addsuffix .h, $(basename $(notdir $(SLICE_SRCS))))
endif

ifneq ($(TEMPLATE_REPOSITORY),)
clean::
	rm -fr $(TEMPLATE_REPOSITORY)
endif
