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
        self.seq = []
        self.seq[0:Demo.seqSize] = range(0, Demo.seqSize)
        self.seq = [0 for x in self.seq]

    def sendByteSeq(self, seq, current=None):
        pass

    def recvByteSeq(self, current=None):
        return self.seq

    def echoByteSeq(self, seq, current=None):
        return self.seq

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
