# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ANT_FLAGS = -emacs
 
ifeq ($(APPEND_VERSION_SUFFIX),yes)
ANT_FLAGS += -Dappend-version-suffix=yes
endif

INSTALL_FLAGS += $(ANT_FLAGS)

ifneq ($(prefix),)
INSTALL_FLAGS += -Dprefix=$(prefix)
endif

ifneq ($(DESTDIR),)
INSTALL_FLAGS += -DDESTDIR=$(DESTDIR)
endif

all:
	ant $(ANT_FLAGS)

clean:
	ant $(ANT_FLAGS) clean

install::
	ant $(INSTALL_FLAGS) install

test::
	@python ./allTests.py
