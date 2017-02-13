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

    custom = Test.CustomPrx.checkedCast(base)
    test(custom)

    byteList = [1, 2, 3, 4, 5]
    if sys.version_info[0] == 2:
        byteString = ''.join(map(chr, byteList))
    else:
        byteString = bytes(byteList)
    stringList = ['s1', 's2', 's3']

    sys.stdout.write("testing custom sequences... ")
    sys.stdout.flush()

    (r, b2) = custom.opByteString1(byteString)
    if sys.version_info[0] == 2:
        test(isinstance(r, str))
        test(isinstance(b2, str))
    else:
        test(isinstance(r, bytes))
        test(isinstance(b2, bytes))
    test(r == byteString)
    test(b2 == byteString)

    (r, b2) = custom.opByteString2(byteString)
    test(isinstance(r, tuple))
    test(isinstance(b2, list))
    for i in range(0, len(byteList)):
        test(r[i] == byteList[i])
        test(b2[i] == byteList[i])

    (r, b2) = custom.opByteList1(byteList)
    test(isinstance(r, list))
    test(isinstance(b2, list))
    for i in range(0, len(byteList)):
        test(r[i] == byteList[i])
        test(b2[i] == byteList[i])

    (r, b2) = custom.opByteList2(byteList)
    if sys.version_info[0] == 2:
        test(isinstance(r, str))
    else:
        test(isinstance(r, bytes))
    test(isinstance(b2, tuple))
    test(r == byteString)
    for i in range(0, len(byteList)):
        test(b2[i] == byteList[i])

    (r, b2) = custom.opStringList1(stringList)
    test(isinstance(r, list))
    test(isinstance(b2, list))
    test(r == stringList)
    test(b2 == stringList)

    (r, b2) = custom.opStringList2(stringList)
    test(isinstance(r, tuple))
    test(isinstance(b2, tuple))
    for i in range(0, len(stringList)):
        test(r[i] == stringList[i])
        test(b2[i] == stringList[i])

    (r, b2) = custom.opStringTuple1(stringList)
    test(isinstance(r, tuple))
    test(isinstance(b2, tuple))
    for i in range(0, len(stringList)):
        test(r[i] == stringList[i])
        test(b2[i] == stringList[i])

    (r, b2) = custom.opStringTuple2(stringList)
    test(isinstance(r, list))
    test(isinstance(b2, list))
    test(r == stringList)
    test(b2 == stringList)

    s = Test.S()
    s.b1 = byteList;
    s.b2 = byteList;
    s.b3 = byteList;
    s.b4 = byteList;
    s.s1 = stringList;
    s.s2 = stringList;
    s.s3 = stringList;
    s.s4 = stringList;
    custom.sendS(s)

    c = Test.C()
    c.b1 = byteList;
    c.b2 = byteList;
    c.b3 = byteList;
    c.b4 = byteList;
    c.s1 = stringList;
    c.s2 = stringList;
    c.s3 = stringList;
    c.s4 = stringList;
    custom.sendC(c)

    print("ok")

    return custom
