#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript

def run(client, server):
    print "testing...",
    sys.stdout.flush()
    client.sendline('1')
    server.expect("Printing string `The streaming API works!'")
    client.sendline('2')
    server.expect("Printing string sequence \\{'The', 'streaming', 'API', 'works!'\\}");
    client.sendline('3')
    server.expect("Printing dictionary \\{")
    i = server.expect(["API=works!", "The=streaming"])
    j = server.expect(["API=works!", "The=streaming"])
    assert i != j
    server.expect("\\}")
    client.sendline('4')
    if demoscript.Util.defaultLanguage == "VB":
        server.expect("Printing enum 1")
        client.sendline('5')
        server.expect("Printing struct: name=red, value=0")
        client.sendline('6')
        server.expect("Printing struct sequence: \\{red=0, green=1, blue=2\\}")
        client.sendline('7')
        server.expect("Printing class: s\\.name=blue, s\\.value=2")
        client.sendline('8')
        client.expect("Got string `hello' and class: s\\.name=green, s\\.value=1")
    else:
        server.expect("Printing enum green")
        client.sendline('5')
        server.expect("Printing struct: name=red, value=red")
        client.sendline('6')
        server.expect("Printing struct sequence: \\{red=red, green=green, blue=blue\\}")
        client.sendline('7')
        server.expect("Printing class: s\\.name=blue, s\\.value=blue")
        client.sendline('8')
        client.expect("Got string `hello' and class: s\\.name=green, s\\.value=green")
    print "ok"

    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
