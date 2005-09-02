#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('--all -I. A.ice')
Ice.loadSlice('--all -I. B.ice')
import Demo

def menu():
    print """
usage:
t: send greeting as twoway
o: send greeting as oneway
O: send greeting as batch oneway
d: send greeting as datagram
D: send greeting as batch datagram
f: flush all batch requests
T: set a timeout
S: switch secure mode on/off
s: shutdown server
x: exit
?: help
"""

def run(args, communicator):
    properties = communicator.getProperties()
#    refProperty = 'Hello.Proxy'
#    proxy = properties.getProperty(refProperty)
#    if len(proxy) == 0:
#        print args[0] + ": property `" + refProperty + "' not set"
#        return False

#    base = communicator.stringToProxy(proxy)
#    twoway = Demo.IAddressPrx.checkedCast(base)
#    if not twoway:
#        print args[0] + ": invalid proxy"
#        return False
#
#    c = None
#    while c != 'x':
#        try:
#            c = raw_input("==> ")
#	    a = twoway.getEnterpriseInfo("111")
#	    print a.name
#        except EOFError:
#            break

    return True

communicator = None
try:
    properties = Ice.createProperties()
    properties.load("config")
    communicator = Ice.initializeWithProperties(sys.argv, properties)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
