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

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise "can't find toplevel directory!"
sys.path.append(path[0])

from demoscript import *
from demoscript.IceBox import hello

if Util.defaultHost:
    args = ' --IceBox.UseSharedCommunicator.IceStorm=1'
else:
    args = ''

iceboxnet = "iceboxnet.exe"
prefix = [ "../../../../cs", "/usr" ]
if os.environ.has_key("ICE_HOME"):
    prefix.append(os.environ["ICE_HOME"])
for p in prefix:
    path = os.path.join(p, "bin", iceboxnet)
    if os.path.exists(path):
        iceboxnet = path
        break

# TODO: This doesn't setup LD_LIBRARY_PATH
server = Util.spawn('%s --Ice.Config=config.icebox --Ice.PrintAdapterReady %s' % (iceboxnet, args))
server.expect('.* ready')
client = Util.spawn('client.exe')
client.expect('.*==>')

hello.run(client, server)
