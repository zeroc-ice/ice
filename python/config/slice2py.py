#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, platform
sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), "..", "python"))
if sys.platform == "win32":
	path = os.getenv('PATH')
	path = path + ';' + os.path.join(os.path.dirname(sys.argv[0]), "..", "..", "cpp", "bin")
	if os.getenv("PLATFORM") == "X64":
		bit, name = platform.architecture()
		if bit == "64bit":
			path = path + ";c:/Program Files (x86)/ZeroC/Ice-3.6.0-ThirdParty/bin/x64"
		else:
			path = path + ";c:/Program Files (x86)/ZeroC/Ice-3.6.0-ThirdParty/bin"
	else:
		path = path + ";c:/Program Files/ZeroC/Ice-3.6.0-ThirdParty/bin"
	os.putenv('PATH', path)

import IcePy

def main():
    val = IcePy.compile(sys.argv)
    sys.exit(int(val))

if __name__ == '__main__':
        main()
