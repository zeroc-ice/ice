#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, platform
sys.path.insert(1, os.path.join(os.path.dirname(sys.argv[0]), "..", "python"))

if sys.platform == "win32":
    os.putenv("PATH", "{0};{1};{2}".format(
        os.getenv('PATH'),
        os.path.join(os.path.dirname(sys.argv[0]), "..", "..", "cpp", "bin"),
        os.path.join(os.path.dirname(sys.argv[0]), "..", "..", "cpp", "third-party-packages",
            "bzip2.v100" if sys.version_info < (3,5) else "bzip2.v140", "build", "native", "bin",
            "Win32" if "32bit" in platform.architecture() else "x64", "Release")))

import IcePy

def main():
    val = IcePy.compile(sys.argv)
    sys.exit(int(val))

if __name__ == '__main__':
        main()
