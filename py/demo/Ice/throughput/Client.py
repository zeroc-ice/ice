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

toggle type of data to send:
1: sequence of bytes (default)
2: sequence of strings (\"hello\")
3: sequence of structs with a string (\"hello\") and a double

select test to run:
t: Send sequence as twoway
o: Send sequence as oneway
r: Receive sequence
e: Echo (send and receive) sequence

other commands:
s: shutdown server
"x: exit
"?: help
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

    byteSeq = []
    byteSeq[0:Demo.ByteSeqSize] = range(0, Demo.ByteSeqSize)
    byteSeq = [0 for x in byteSeq]

    stringSeq = []
    stringSeq[0:Demo.StringSeqSize] = range(0, Demo.StringSeqSize)
    stringSeq = ["hello" for x in stringSeq]

    structSeq = []
    structSeq[0:Demo.StringDoubleSeqSize] = range(0, Demo.StringDoubleSeqSize)
    for i in range(0, Demo.StringDoubleSeqSize):
    	structSeq[i] = Demo.StringDouble()
        structSeq[i].s = "hello"
	structSeq[i].d = 3.14

    menu()

    throughput.ice_ping() # Initial ping to setup the connection.

    currentType = '1'
    seqSize = Demo.ByteSeqSize

    c = None
    while c != 'x':
        try:
            c = raw_input("==> ")

            repetitions = 100

	    if c == '1' or c == '2' or c == '3':
	        currentType = c
		if c == '1':
		    print "using byte sequences"
		    seqSize = Demo.ByteSeqSize
		elif c == '2':
		    print "using string sequences"
		    seqSize = Demo.StringSeqSize
		elif c == '3':
		    print "using struct sequences"
		    seqSize = Demo.StringDoubleSeqSize
            elif c == 't' or c == 'o' or c == 'r' or c == 'e':
                if c == 't' or c == 'o':
                    print "sending",
                elif c == 'r':
                    print "receiving",
                elif c == 'e':
                    print "sending and receiving",

		print repetitions,
		if currentType == '1':
		    print "byte",
		elif currentType == '2':
		    print "string",
		elif currentType == '3':
		    print "double",
		
                if c == 'o':
                    print "sequences of size %d as oneway..." % seqSize
                else:
                    print "sequences of size %d..." % seqSize

                tsec = time.time()

                for i in range(0, repetitions):
		    if currentType == '1':
                        if c == 't':
                            throughput.sendByteSeq(byteSeq)
                        elif c == 'o':
                            throughputOneway.sendByteSeq(byteSeq)
                        elif c == 'r':
                            throughput.recvByteSeq()
                        elif c == 'e':
                            throughput.echoByteSeq(byteSeq)
		    elif currentType == '2':
                        if c == 't':
                            throughput.sendStringSeq(stringSeq)
                        elif c == 'o':
                            throughputOneway.sendStringSeq(stringSeq)
                        elif c == 'r':
                            throughput.recvStringSeq()
                        elif c == 'e':
                            throughput.echoStringSeq(stringSeq)
		    elif currentType == '3':
                        if c == 't':
                            throughput.sendStructSeq(structSeq)
                        elif c == 'o':
                            throughputOneway.sendStructSeq(structSeq)
                        elif c == 'r':
                            throughput.recvStructSeq()
                        elif c == 'e':
                            throughput.echoStructSeq(structSeq)

                tsec = time.time() - tsec
                tmsec = tsec * 1000.0
                print "time for %d sequences: %.3fms" % (repetitions, tmsec)
                print "time per sequence: %.3fms" % (tmsec / repetitions)
		wireSize = 0
		if currentType == '1':
		    wireSize = 1
		elif currentType == '2':
		    wireSize = len(stringSeq[0])
		elif currentType == '3':
		    wireSize = len(structSeq[0].s)
		    wireSize += 8
                mbit = repetitions * seqSize * wireSize * 8.0 / tsec / 1000000.0
                if c == 'e':
                    mbit = mbit * 2
                print "throughput: %.3f MBit/s" % mbit
            elif c == 's':
	        throughput.shutdown()
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
