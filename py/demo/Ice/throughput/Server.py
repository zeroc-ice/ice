#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

        self.fixedSeq = []
        self.fixedSeq[0:Demo.FixedSeqSize] = range(0, Demo.FixedSeqSize)
	for i in range(0, Demo.FixedSeqSize):
	    self.fixedSeq[i] = Demo.Fixed()
	    self.fixedSeq[i].i = 0
	    self.fixedSeq[i].j = 0
	    self.fixedSeq[i].d = 0.0

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

    def sendFixedSeq(self, seq, current=None):
        pass

    def recvFixedSeq(self, current=None):
        return self.fixedSeq

    def echoFixedSeq(self, seq, current=None):
        return seq

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
	adapter = self.communicator().createObjectAdapter("Throughput")
	adapter.add(ThroughputI(), Ice.stringToIdentity("throughput"))
	adapter.activate()
	self.communicator().waitForShutdown()
	return True

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
