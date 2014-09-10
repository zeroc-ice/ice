# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Determines whether the extension uses PHP namespaces (requires
# PHP 5.3 or later).
#
#USE_NAMESPACES          ?= yes

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = php
slice_translator = slice2php

ifeq ($(shell test -f $(top_srcdir)/config/Make.common.rules && echo 0),0)
    include $(top_srcdir)/config/Make.common.rules
else
    include $(top_srcdir)/../config/Make.common.rules
endif

#
# Platform specific definitions
#
ifeq ($(shell test -f $(top_srcdir)/config/Make.rules.$(UNAME) && echo 0),0)
    configdir = $(top_srcdir)/config
else
    configdir = $(top_srcdir)/../cpp/config
endif 
include	$(configdir)/Make.rules.$(UNAME)

ICECPPFLAGS		= -I$(slicedir)
SLICE2PHPFLAGS		= $(ICECPPFLAGS)

ifeq ("$(USE_NAMESPACES)","yes")
    SLICE2PHPFLAGS      := $(SLICE2PHPFLAGS) -n
endif

ifdef ice_src_dist
    ifeq ($(ice_cpp_dir), $(ice_dir)/cpp)
        SLICE2PHP 	= $(ice_cpp_dir)/bin/slice2php
    else
        SLICE2PHP 	= $(ice_cpp_dir)/$(binsubdir)/slice2php
    endif
else
    SLICE2PHP 		= $(ice_dir)/$(binsubdir)/slice2php
endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.php .ice

ifneq ($(GEN_SRCS),)

all:: $(GEN_SRCS)

clean::
    -rm -f $(GEN_SRCS)
    -rm -f .depend/*.d

-include $(addprefix .depend/, $(GEN_SRCS:.php=.ice.d))

endif

%.php: $(SDIR)/%.ice
	rm -f $(*F).php
	$(SLICE2PHP) $(SLICE2PHPFLAGS) $<
    @mkdir -p .depend
    @$(SLICE2PHP) $(SLICE2PHPFLAGS) --depend $< > .depend/$(*F).ice.d

clean::
	-rm -f $(TARGETS)
	-rm -f core *.o *.bak

all:: $(TARGETS)

install::
