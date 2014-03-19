# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

all:
	ant -emacs

clean:
	ant -emacs clean

install::
!if "$(prefix)" != ""
	ant -emacs -Dprefix="$(prefix)" install
!else
	ant -emacs install
!endif

test:
	@python .\allTests.py
