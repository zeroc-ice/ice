#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../..", "../../../../../..",
         "../../../../../../..", "../../../../../../../..", "../../../../../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil, NetworkProxy

sys.stdout.write("starting SOCKS proxy... ")
sys.stdout.flush()
socksProxy = NetworkProxy.SocksProxy(12030)
print("ok")

TestUtil.clientServerTest(additionalClientOptions="--Ice.SOCKSProxyHost=localhost --Ice.SOCKSProxyPort=12030")

sys.stdout.write("terminating SOCKS proxy... ")
sys.stdout.flush()
socksProxy.terminate()
print("ok")

sys.stdout.write("starting HTTP proxy... ")
sys.stdout.flush()
httpProxy = NetworkProxy.HttpProxy(12031)
print("ok")

TestUtil.clientServerTest(additionalClientOptions="--Ice.HTTPProxyHost=localhost --Ice.HTTPProxyPort=12031")

sys.stdout.write("terminating HTTP proxy... ")
sys.stdout.flush()
httpProxy.terminate()
print("ok")
