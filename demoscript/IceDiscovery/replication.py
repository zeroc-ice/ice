#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
import Expect

def runtests(client, serverCount):
    sys.stdout.write("testing " + str(serverCount) + " server(s)...")
    sys.stdout.flush()
    client.sendline('5')
    client.expect('.*enter the delay between each greetings \(in ms\):.*')
    client.sendline('50')
    if serverCount == 0:
        client.expect('.*NoEndpointException.*')
    else:
    	for i in range (0, 5):
    	    client.expect('Server. says Hello World!')
    print("ok")

def run(client, server1, server2, server3):
    runtests(client, 3)

    print("shutdown first server");
    server1.terminate()
    runtests(client, 2)

    print("shutdown second server");
    server2.terminate()
    runtests(client, 1)

    print("shutdown third server");
    server3.terminate()
    runtests(client, 0)

    client.terminate()
