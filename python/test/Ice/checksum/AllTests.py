# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, string, re, traceback, Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    ref = "test:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)

    checksum = Test.ChecksumPrx.checkedCast(base)
    test(checksum)

    #
    # Verify that no checksums are present for local types.
    #
    sys.stdout.write("testing checksums... ")
    sys.stdout.flush()
    test(len(Ice.sliceChecksums) > 0)
    for i in Ice.sliceChecksums:
        test(i.find("Local") == -1)

    #
    # Get server's Slice checksums.
    #
    d = checksum.getSliceChecksums()

    #
    # Compare the checksums. For a type FooN whose name ends in an integer N,
    # we assume that the server's type does not change for N = 1, and does
    # change for N > 1.
    #
    patt = re.compile("\\d+")
    for i in d:
        n = 0
        m = patt.search(i)
        if m:
            n = int(i[m.start():])

        test(i in Ice.sliceChecksums)

        if n <= 1:
            test(Ice.sliceChecksums[i] == d[i])
        else:
            test(Ice.sliceChecksums[i] != d[i])

    print("ok")

    return checksum
