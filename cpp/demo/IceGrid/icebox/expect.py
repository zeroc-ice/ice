#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(path[0])

from demoscript import Util
from demoscript.IceGrid import icebox

if Util.isDarwin():
    #
    # On OS X, make sure to also run the IceBox services in 32bits mode if
    # x64 isn't specified and the service is built for 32bits.
    #
    iceBox = os.path.join(os.getcwd(), "iceboxwrapper")
    iceBoxWrapper = open(iceBox, "w")
    if Util.x64:
        iceBoxWrapper.write("#!/bin/sh\narch -x86_64 icebox \"$@\"\n")
    else:
        iceBoxWrapper.write("#!/bin/sh\narch -i386 -x86_64 icebox \"$@\"\n")
    iceBoxWrapper.close()
    os.chmod(iceBox, 0o700)

desc = 'application.xml'
if Util.isDebugBuild() or Util.isDarwin():
    fi = open(desc, "r")
    desc = 'tmp_application.xml'
    fo = open(desc, "w")
    for l in fi:
        if l.find('exe="icebox"') != -1:
            if Util.isDarwin():
                l = l.replace('exe="icebox"', 'exe="./iceboxwrapper"')
            else:
                l = l.replace('exe="icebox"', 'exe="iceboxd.exe"')
        fo.write(l)
    fi.close()
    fo.close()

Util.addLdPath(os.getcwd())

icebox.run('./client', desc[0:len(desc)-4])

if Util.isDarwin():
    os.unlink("iceboxwrapper")
