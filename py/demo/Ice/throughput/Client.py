#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, traceback, Ice

Ice.loadSlice('Throughput.ice')
import Demo

def menu():
    print """
usage:
s: send byte sequence
o: send byte sequence as oneway
r: receive byte sequence
e: echo (send and receive) byte sequence
x: exit
?: help
"""

def run(args, communicator):
    properties = communicator.getProperties()
    proxyProperty = 'Throughput.Throughput'
    proxy = properties.getProperty(proxyProperty)
    if len(proxy) == 0:
        print args[0] + ": property `" + proxyProperty + "' not set"
        return False

    base = communicator.stringToProxy(proxy)
    throughput = Demo.ThroughputPrx.checkedCast(base)
    if not throughput:
        print args[0] + ": invalid proxy"
        return False
    throughputOneway = Demo.ThroughputPrx.uncheckedCast(throughput.ice_oneway())

    seq = []
    seq[0:Demo.seqSize] = range(0, Demo.seqSize)
    seq = [0 for x in seq]

    menu()

    c = None
    while c != 'x':
        try:
            c = raw_input("==> ")

            throughput.ice_ping() # Initial ping to setup the connection.

            repetitions = 100

            if c == 's' or c == 'o' or c == 'r' or c == 'e':
                if c == 's' or c == 'o':
                    print "sending",
                elif c == 'r':
                    print "receiving",
                elif c == 'e':
                    print "sending and receiving",

                if c == 'o':
                    print repetitions, "sequences of size %d as oneway..." % Demo.seqSize
                else:
                    print repetitions, "sequences of size %d..." % Demo.seqSize

                tsec = time.time()

                for i in range(0, repetitions):
                    if c == 's':
                        throughput.sendByteSeq(seq)
                    elif c == 'o':
                        throughputOneway.sendByteSeq(seq)
                    elif c == 'r':
                        throughput.recvByteSeq()
                    elif c == 'e':
                        throughput.echoByteSeq(seq)

                tsec = time.time() - tsec
                tmsec = tsec * 1000.0
                print "time for %d sequences: %.3fms" % (repetitions, tmsec)
                print "time per sequence: %.3fms" % (tmsec / repetitions)
                mbit = repetitions * Demo.seqSize * 8.0 / tsec / 1000000.0
                if c == 'e':
                    mbit = mbit * 2
                print "throughput: %.3f MBit/s" % mbit
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
