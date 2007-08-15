#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys

def run(client, server):
    print "testing...",
    sys.stdout.flush()
    client.expect('Created README')
    server.expect('added')
    client.expect('Contents of filesystem:')
    server.expect('locate')
    client.expect('Contents of file')
    server.expect('locate')
    client.expect('Down to a sunless sea')
    server.expect('locate')
    i = client.expect(['Destroying Coleridge', 'Destroying README'])
    server.expect('removed object')
    j = client.expect(['Destroying Coleridge', 'Destroying README'])
    assert i != j
    server.expect('removed object')
    client.expect(pexpect.EOF)
    assert client.wait() == 0
    print "ok"
