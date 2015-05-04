#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice, IcePy
def main():
    sliceDir = Ice.getSliceDir()
    # Automatically add the slice dir.
    if not sliceDir is None:
        sys.argv.append('-I' + sliceDir)
    val = IcePy.compile(sys.argv)
    sys.exit(int(val))
