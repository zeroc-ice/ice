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
    print "makeIceSpec -t [suse|redhat] -v [ice version]"
    print "where:"
    print "     -t, --target   one of suse or redhat"
    print "     -v, --version  the ice version number for the spec file"

def main():
    #
    # targetSystem describes the system we are building the SRPM for.
    # Different target systems will have different contents + build
    # steps.
    #
    targetSystem = None

    #
    # Ice version
    #
    iceVersion = None

    try:
        optionList, args = getopt.getopt(sys.argv[1:], "hs:t:v:", ["target=", "help", "version"])

    except getopt.GetoptError:
        usage()
        sys.exit(2)

    for o, a in optionList:
        if o in ["-h", "--help"]:
            usage()
            sys.exit(0)
        elif o in ["-t", "--target-host"]:
            targetSystem = a
        elif o in ["-v", "--version"]:
            iceVersion = a

    if iceVersion == None or targetSystem == None:
        print "must specify Ice version with and target system"
        usage()
        sys.exit(1)

    if not targetSystem in ["suse", "redhat"]:
        print "target distro must be 'suse' or 'redhat'!"
        sys.exit(1)

    verComponents = iceVersion.split(".")
    libVersion = verComponents[0] + verComponents[1]

    f = open("./ice-%s-%s.spec" % (iceVersion, targetSystem), "w+b")
    RPMTools.createFullSpecFile(f, "", iceVersion, int(libVersion), targetSystem)
    f.close()

if __name__ == "__main__":
    main()
