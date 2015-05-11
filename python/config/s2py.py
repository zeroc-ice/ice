#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, platform
sys.path.insert(1, os.path.join(os.path.dirname(sys.argv[0]), "..", "python"))
if sys.platform == "win32":
        path = os.getenv('PATH')
        path = path + ';' + os.path.join(os.path.dirname(sys.argv[0]), "..", "..", "cpp", "bin")

        programFiles = "ProgramFiles"
        arch1 = os.getenv('PROCESSOR_ARCHITECTURE')
        arch2 = os.getenv('PROCESSOR_ARCHITEW6432')
        
        # 64 bit windows machine?
        if arch1 == "AMD64" or arch1 == "IA64" or arch2 == "AMD64" or arch2 == "IA64":
            programFiles += "(x86)"
            bit, name = platform.architecture()
            # 64 bit python?
            if bit == "64bit":
                suffix = "x64"
        path = path + ";" + os.path.join(os.getenv(programFiles), "ZeroC", "Ice-3.6.0-ThirdParty","bin", suffix)
        os.putenv('PATH', path)

import IcePy

def main():
    val = IcePy.compile(sys.argv)
    sys.exit(int(val))

if __name__ == '__main__':
        main()
