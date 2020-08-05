#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys
import Ice
import IcePy


def main():
    sliceDir = Ice.getSliceDir()
    # Automatically add the slice dir.
    if sliceDir is not None:
        sys.argv.append('-I' + sliceDir)
    val = IcePy.compile(sys.argv)
    sys.exit(int(val))


if __name__ == "__main__":
    sys.exit(main())
