#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        warmup = False
        
        if sys.version_info[0] == 2:
            b = []
            b[0:Demo.ByteSeqSize] = range(0, Demo.ByteSeqSize)
            b = ['\x00' for x in b]
            self.byteSeq = ''.join(b)
        else:
            self.byteSeq = bytes([0 for x in range(0, Demo.ByteSeqSize)])

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

    def needsWarmup(self, current=None):
        self.warmup = False
        return False

    def startWarmup(self, current=None):
        self.warmup = True

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
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        adapter = self.communicator().createObjectAdapter("Throughput")
        adapter.add(ThroughputI(), self.communicator().stringToIdentity("throughput"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
