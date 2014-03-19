#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, traceback, Ice

Ice.loadSlice('Throughput.ice')
import Demo

def menu():
    print("""
usage:

toggle type of data to send:
1: sequence of bytes (default)
2: sequence of strings (\"hello\")
3: sequence of structs with a string (\"hello\") and a double
4: sequence of structs with two ints and a double

select test to run:
t: Send sequence as twoway
o: Send sequence as oneway
r: Receive sequence
e: Echo (send and receive) sequence

other commands:
s: shutdown server
x: exit
?: help
""")

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        throughput = Demo.ThroughputPrx.checkedCast(self.communicator().propertyToProxy('Throughput.Proxy'))
        if not throughput:
            print(args[0] + ": invalid proxy")
            return 1
        throughputOneway = Demo.ThroughputPrx.uncheckedCast(throughput.ice_oneway())

        byteSeq = bytearray(Demo.ByteSeqSize)

        stringSeq = []
        stringSeq[0:Demo.StringSeqSize] = range(0, Demo.StringSeqSize)
        stringSeq = ["hello" for x in stringSeq]

        structSeq = []
        structSeq[0:Demo.StringDoubleSeqSize] = range(0, Demo.StringDoubleSeqSize)
        for i in range(0, Demo.StringDoubleSeqSize):
            structSeq[i] = Demo.StringDouble()
            structSeq[i].s = "hello"
            structSeq[i].d = 3.14

        fixedSeq = []
        fixedSeq[0:Demo.FixedSeqSize] = range(0, Demo.FixedSeqSize)
        for i in range(0, Demo.FixedSeqSize):
            fixedSeq[i] = Demo.Fixed()
            fixedSeq[i].i = 0
            fixedSeq[i].j = 0
            fixedSeq[i].d = 0.0

        #
        # To allow cross-language tests we may need to "warm up" the
        # server. The warm up is to ensure that any JIT compiler will
        # have converted any hotspots to native code. This ensures an
        # accurate throughput measurement.
        #
        if throughput.needsWarmup():
            throughput.startWarmup()

            emptyBytes = [ '\x00' ]
            emptyBytes = ''.join(emptyBytes)
            emptyStrings = [ "" ]
            emptyStructs = [ Demo.StringDouble() ]
            emptyFixed = [ Demo.Fixed() ]

            print("warming up the server...",)
            sys.stdout.flush()
            for i in range(0, 10000):
                throughput.sendByteSeq(emptyBytes)
                throughput.sendStringSeq(emptyStrings)
                throughput.sendStructSeq(emptyStructs)
                throughput.sendFixedSeq(emptyFixed)

                throughput.recvByteSeq()
                throughput.recvStringSeq()
                throughput.recvStructSeq()
                throughput.recvFixedSeq()

                throughput.echoByteSeq(emptyBytes)
                throughput.echoStringSeq(emptyStrings)
                throughput.echoStructSeq(emptyStructs)
                throughput.echoFixedSeq(emptyFixed)

            throughput.endWarmup()

            print("ok")
        else:
            throughput.ice_ping() # Initial ping to setup the connection.

        menu()

        currentType = '1'
        seqSize = Demo.ByteSeqSize

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()

                repetitions = 100

                if c == '1' or c == '2' or c == '3' or c == '4':
                    currentType = c
                    if c == '1':
                        print("using byte sequences")
                        seqSize = Demo.ByteSeqSize
                    elif c == '2':
                        print("using string sequences")
                        seqSize = Demo.StringSeqSize
                    elif c == '3':
                        print("using variable-length struct sequences")
                        seqSize = Demo.StringDoubleSeqSize
                    elif c == '4':
                        print("using fixed-length struct sequences")
                        seqSize = Demo.FixedSeqSize
                elif c == 't' or c == 'o' or c == 'r' or c == 'e':
                    if c == 't' or c == 'o':
                        sys.stdout.write("sending ")
                    elif c == 'r':
                        sys.stdout.write("receiving ")
                    elif c == 'e':
                        sys.stdout.write("sending and receiving ")

                    sys.stdout.write(str(repetitions) + " ")
                    if currentType == '1':
                        sys.stdout.write("byte ")
                    elif currentType == '2':
                        sys.stdout.write("string ")
                    elif currentType == '3':
                        sys.stdout.write("variable-length struct ")
                    elif currentType == '4':
                        sys.stdout.write("fixed-length struct ")

                    if c == 'o':
                        print("sequences of size %d as oneway..." % seqSize)
                    else:
                        print("sequences of size %d..." % seqSize)

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
                        elif currentType == '4':
                            if c == 't':
                                throughput.sendFixedSeq(fixedSeq)
                            elif c == 'o':
                                throughputOneway.sendFixedSeq(fixedSeq)
                            elif c == 'r':
                                throughput.recvFixedSeq()
                            elif c == 'e':
                                throughput.echoFixedSeq(fixedSeq)

                    tsec = time.time() - tsec
                    tmsec = tsec * 1000.0
                    print("time for %d sequences: %.3fms" % (repetitions, tmsec))
                    print("time per sequence: %.3fms" % (tmsec / repetitions))
                    wireSize = 0
                    if currentType == '1':
                        wireSize = 1
                    elif currentType == '2':
                        wireSize = len(stringSeq[0])
                    elif currentType == '3':
                        wireSize = len(structSeq[0].s)
                        wireSize += 8
                    elif currentType == '4':
                        wireSize = 16
                    mbit = repetitions * seqSize * wireSize * 8.0 / tsec / 1000000.0
                    if c == 'e':
                        mbit = mbit * 2
                    print("throughput: %.3fMbps" % mbit)
                elif c == 's':
                    throughput.shutdown()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print("unknown command `" + c + "'")
                    menu()
            except EOFError:
                break
            except KeyboardInterrupt:
                break

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
