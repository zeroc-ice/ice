#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(path[0])

from demoscript import Util

server = Util.spawn('./workqueue')
server.expect('Pushing work items')

sys.stdout.write("testing... ")
sys.stdout.flush()
server.expect('work item: item1')
server.expect('work item: item2')
server.expect('work item: item3')
server.expect('work item: item4')
server.expect('work item: item5')
server.waitTestSuccess(timeout=10)
print("ok")
