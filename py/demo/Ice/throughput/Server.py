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

Ice.loadSlice('Throughput.ice')
import Demo

class ThroughputI(Demo.Throughput):
    def __init__(self):
        self.byteSeq = []
        self.byteSeq[0:Demo.ByteSeqSize] = range(0, Demo.ByteSeqSize)
        self.byteSeq = [0 for x in self.byteSeq]

        self.stringSeq = []
        self.stringSeq[0:Demo.StringSeqSize] = range(0, Demo.StringSeqSize)
        self.stringSeq = ["hello" for x in self.stringSeq]

        self.structSeq = []
        self.structSeq[0:Demo.StringDoubleSeqSize] = range(0, Demo.StringDoubleSeqSize)
	for i in range(0, Demo.StringDoubleSeqSize):
	    self.structSeq[i] = Demo.StringDouble()
	    self.structSeq[i].s = "hello"
	    self.structSeq[i].d = 3.14

    def sendByteSeq(self, seq, current=None):
        pass

    def recvByteSeq(self, current=None):
        return self.byteSeq

    def echoByteSeq(self, seq, current=None):
        return seq

    def sendStringSeq(self, seq, current=None):
        pass

    def recvStringSeq(self, current=None):
        return self.stringSeq

    def echoStringSeq(self, seq, current=None):
        return seq

    def sendStructSeq(self, seq, current=None):
        pass

    def recvStructSeq(self, current=None):
        return self.structSeq

    def echoStructSeq(self, seq, current=None):
        return seq

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

def run(argv, communicator):
    adapter = communicator.createObjectAdapter("Throughput")
    object = ThroughputI()
    adapter.add(object, Ice.stringToIdentity("throughput"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

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
        print ex
        status = False

if status:
    sys.exit(0)
else:
    sys.exit(1)
