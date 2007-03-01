#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        warmup = True
        
        bytes = []
        bytes[0:Demo.ByteSeqSize] = range(0, Demo.ByteSeqSize)
        bytes = ['\x00' for x in bytes]
        self.byteSeq = ''.join(bytes)

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

    def endWarmup(self, current=None):
        self.warmup = False
        
    def sendByteSeq(self, seq, current=None):
        pass

    def recvByteSeq(self, current=None):
        if self.warmup:
            return []
        else:
            return self.byteSeq

    def echoByteSeq(self, seq, current=None):
        return seq

    def sendStringSeq(self, seq, current=None):
        pass

    def recvStringSeq(self, current=None):
        if self.warmup:
            return []
        else:
            return self.stringSeq

    def echoStringSeq(self, seq, current=None):
        return seq

    def sendStructSeq(self, seq, current=None):
        pass

    def recvStructSeq(self, current=None):
        if self.warmup:
            return []
        else:
            return self.structSeq

    def echoStructSeq(self, seq, current=None):
        return seq

    def sendFixedSeq(self, seq, current=None):
        pass

    def recvFixedSeq(self, current=None):
        if self.warmup:
            return []
        else:
            return self.fixedSeq

    def echoFixedSeq(self, seq, current=None):
        return seq

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("Throughput")
        adapter.add(ThroughputI(), self.communicator().stringToIdentity("throughput"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
