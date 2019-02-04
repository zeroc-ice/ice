#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, Ice, IcePy
def main():
    sliceDir = Ice.getSliceDir()
    # Automatically add the slice dir.
    if not sliceDir is None:
        sys.argv.append('-I' + sliceDir)
    val = IcePy.compile(sys.argv)
    sys.exit(int(val))
