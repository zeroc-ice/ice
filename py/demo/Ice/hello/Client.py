# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Hello, Ice, _Top

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
    proxy = properties.getProperty('Hello.Proxy')
    if len(proxy) == 0:
        print " property `Hello.Proxy' not set"
        return False

    base = communicator.stringToProxy(proxy)
    twoway = _Top.HelloPrx.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(False))
    if not twoway:
        print "invalid proxy"
        return False

    oneway = _Top.HelloPrx.uncheckedCast(twoway.ice_oneway())
    batchOneway = _Top.HelloPrx.uncheckedCast(twoway.ice_batchOneway())
    datagram = _Top.HelloPrx.uncheckedCast(twoway.ice_datagram())
    batchDatagram = _Top.HelloPrx.uncheckedCast(twoway.ice_batchDatagram())

    secure = False
    timeout = -1

    menu()

    c = None
    while c != 'x':
        try:
            c = raw_input("==> ")
            if c == 't':
                twoway.sayHello()
            elif c == 'o':
                oneway.sayHello()
            elif c == 'O':
                batchOneway.sayHello()
            elif c == 'd':
                if secure:
                    print "secure datagrams are not supported"
                else:
                    datagram.sayHello()
            elif c == 'D':
                if secure:
                    print "secure datagrams are not supported"
                else:
                    batchDatagram.sayHello()
            elif c == 'f':
                communicator.flushBatchRequests()
            elif c == 'T':
                if timeout == -1:
                    timeout = 2000
                else:
                    timeout = -1

                twoway = _Top.HelloPrx.uncheckedCast(twoway.ice_timeout(timeout))
                oneway = _Top.HelloPrx.uncheckedCast(oneway.ice_timeout(timeout))
                batchOneway = _Top.HelloPrx.uncheckedCast(batchOneway.ice_timeout(timeout))

                if timeout == -1:
                    print "timeout is now switched off"
                else:
                    print "timeout is now set to 2000ms"
            elif c == 'S':
                secure = not secure

                twoway = _Top.HelloPrx.uncheckedCast(twoway.ice_secure(secure))
                oneway = _Top.HelloPrx.uncheckedCast(oneway.ice_secure(secure))
                batchOneway = _Top.HelloPrx.uncheckedCast(batchOneway.ice_secure(secure))
                datagram = _Top.HelloPrx.uncheckedCast(datagram.ice_secure(secure))
                batchDatagram = _Top.HelloPrx.uncheckedCast(batchDatagram.ice_secure(secure))

                if secure:
                    print "secure mode is now on"
                else:
                    print "secure mode is now off"
            elif c == 's':
                twoway.shutdown()
            elif c == 'x':
                pass # Nothing to do
            elif c == '?':
                menu()
            else:
                print "unknown command `" + c + "'"
                menu()
        except EOFError:
            break

    return True

try:
    properties = Ice.createProperties()
    properties.load("config")
    communicator = Ice.initializeWithProperties(sys.argv, properties)
    status = run(sys.argv, communicator)
except Ice.Exception, ex:
    print ex
    status = False

if communicator:
    try:
        communicator.destroy()
    except Ice.Exception, ex:
        print ex
        status = False

if status:
    sys.exit(0)
else:
    sys.exit(1)
