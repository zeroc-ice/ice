#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

def isWindows():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    if sys.platform == "win32" or sys.platform[:6] == "cygwin":
        return True
    return False

def findHome():
    cahome = os.getenv("ICE_CA_HOME")
    if cahome is None:
	if isWindows():
	    cahome = os.path.dirname(sys.argv[0])
	    cahome = os.path.join(cahome, "config", "ca")
	else:
	    cahome = os.getenv('HOME')
	    if cahome is None:
		print "Environment variable HOME is not set."
		sys.exit(1)
	    cahome = os.path.join(cahome, ".iceca")

    os.putenv("ICE_CA_HOME", cahome)

    return os.path.normpath(cahome)
