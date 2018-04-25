#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, platform

basepath = os.path.dirname(os.path.realpath(__file__))

if sys.platform == "win32":
    platformName = "Win32" if "32bit" in platform.architecture() else "x64"
    configurationName = os.getenv("CPP_CONFIGURATION", "Release")
    sys.path.insert(1, os.path.join(basepath, "..", "python", platformName, configurationName))
    os.putenv("PATH", os.path.join(basepath, "..", "..", "cpp", "bin", platformName, configurationName))
else:
    sys.path.insert(1, os.path.join(basepath, "..", "python"))

import IcePy

def main():
    val = IcePy.compile(sys.argv)
    sys.exit(int(val))

if __name__ == '__main__':
        main()
