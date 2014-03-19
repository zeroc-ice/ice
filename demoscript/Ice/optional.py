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

def run(client, server):
    sys.stdout.write("testing... ")
    sys.stdout.flush()

    client.expect('Checking john... ok')
    client.expect('Checking steve... ok')
    client.expect('Checking frank... ok')
    client.expect('Checking anne... ok')

    server.waitTestSuccess()
    client.waitTestSuccess()
    print("ok")
