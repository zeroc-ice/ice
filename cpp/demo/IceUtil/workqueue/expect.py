#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

try:
    import demoscript
except ImportError:
    for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
        toplevel = os.path.normpath(toplevel)
        if os.path.exists(os.path.join(toplevel, "demoscript")):
            break
    else:
        raise "can't find toplevel directory!"
    sys.path.append(os.path.join(toplevel))
    import demoscript

import demoscript.Util
demoscript.Util.defaultLanguage = "C++"
import demoscript.Ice.hello

server = demoscript.Util.spawn('./workqueue')
server.expect('Pushing work items')

print "testing...",
sys.stdout.flush()
server.expect('work item: item1')
server.expect('work item: item2')
server.expect('work item: item3')
server.expect('work item: item4')
server.expect('work item: item5')
server.waitTestSuccess(timeout=10)
print "ok"
