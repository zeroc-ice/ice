#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
import Ice
import IcePy


def main():
    sliceDir = Ice.getSliceDir()
    # Automatically add the slice dir.
    if sliceDir is not None:
        sys.argv.append("-I" + sliceDir)
    val = IcePy.compileSlice(sys.argv)
    return int(val)


if __name__ == "__main__":
    sys.exit(main())
