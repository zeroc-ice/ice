#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
import os, sys, getopt
import RPMTools

def usage():
    pass

def main():
    #
    # targetSystem describes the system we are building the SRPM for.
    # Different target systems will have different contents + build
    # steps.
    #
    targetSystem = None

    #
    # sourceLocation indicates a directory that contains the required
    # SRPM contents.
    #
    sourceLocation = None

    try:
	optionList, args = getopt.getopt(sys.argv[1:], "hs:t:", ["sources=", "target-host=", "help"])

    except getopt.GetoptError:
	usage()
	sys.exit(2)

    for o, a in optionList:
	if o in ["-h", "--help"]:
	    pass
	if o in ["-s", "--sources"]:
	    pass
	if o in ["-t", "--target-host"]:
	    pass

    f = open("./test.spec", "w+b")
    RPMTools.createFullSpecFile(f, "", "3.2.0", 32, "suse")
    f.close()

if __name__ == "__main__":
    main()
