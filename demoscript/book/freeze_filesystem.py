#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, signal

def run(client, server):
    print "testing...",
    sys.stdout.flush()
    client.expect('Created Coleridge/Kubla_Khan')
    server.expect('added')
    client.expect('Contents of filesystem:')
    server.expect('added')
    server.expect('added')
    server.expect('locate')
    server.expect('locate')
    client.expect('Down to a sunless sea')
    server.expect('locate')
    client.expectall(['Destroying Coleridge', 'Destroying README'])
    server.expect('removed object')
    server.expect('removed object')
    client.waitTestSuccess()
    print "ok"

    server.kill(signal.SIGINT)
    server.waitTestSuccess()
