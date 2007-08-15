#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys, os

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

if os.path.exists("../../../../cpp"):
    iceHome = "../../../../cpp"
elif os.path.exists("../../../demo"):
    iceHome = "../../../"
else:
    print "Cannot find C++ demos"
    sys.exit(1)

import demoscript.Util

cwd = os.getcwd()
os.chdir('%s/demo/Ice/minimal' % (iceHome))
server = demoscript.Util.spawn('./server --Ice.PrintAdapterReady')
server.expect('.* ready')
os.chdir(cwd)

print "testing...",
sys.stdout.flush()
client = demoscript.Util.spawn('ruby Client.rb')
client.expect(pexpect.EOF)
server.expect('Hello World!')
print "ok"
