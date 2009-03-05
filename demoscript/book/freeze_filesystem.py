#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    client.expect('Contents of filesystem:')
    client.expect('Down to a sunless sea')
    client.expectall(['Destroying Coleridge', 'Destroying README'])
    server.expect('removed object')
    server.expect('removed object')
    server.expect('removed object')
    client.waitTestSuccess()
    print "ok"

    server.kill(signal.SIGINT)
    server.waitTestSuccess()
