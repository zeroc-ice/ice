#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys

def run(client, isJava=False):
    if isJava:
        client.expect('IntIntMap \\(Collections API\\)')
        print("IntIntMap (Collections API):")
        client.expect('IntIntMap \\(Fast API\\)', timeout=200)
        print("%s " % (client.before))

        print("IntIntMap (Fast API):")
        client.expect('IntIntMap with index \\(Collections API\\)', timeout=200)
        print("%s " % (client.before))

        print("IntIntMap with index (Collections API):")
        client.expect('IntIntMap with index \\(Fast API\\)', timeout=200)
        print("%s " % (client.before))

        print("IntIntMap with index (Fast API):")
        client.expect('Struct1Struct2Map', timeout=200)
        print("%s " % (client.before))
    else:
        client.expect('IntIntMap')
        print("IntIntMap:")
        client.expect('IntIntMap with index', timeout=200)
        print("%s " % (client.before))

        print("IntIntMap with index:")
        client.expect('Struct1Struct2Map', timeout=200)
        print("%s " % (client.before))

    print("Struct1Struct2Map:")
    client.expect('Struct1Struct2Map with index', timeout=200)
    print("%s " % (client.before))

    print("Struct1Struct2Map with index:")
    client.expect('Struct1Class1Map', timeout=200)
    print("%s " % (client.before))

    print("Struct1Class1Map:")
    client.expect('Struct1Class1Map with index', timeout=200)
    print("%s " % (client.before))

    print("Struct1Class1Map with index:")
    client.expect('Struct1ObjectMap', timeout=200)
    print("%s " % (client.before))

    print("Struct1ObjectMap:")
    client.expect('IntIntMap \\(read test\\)', timeout=200)
    print("%s " % (client.before))

    print("IntIntMap (read test):")
    client.expect('IntIntMap with index \\(read test\\)', timeout=200)
    print("%s " % (client.before))

    print("IntIntMap with index (read test):")
    client.waitTestSuccess(timeout=200)
    print("%s " % (client.before))
