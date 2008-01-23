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
demoscript.Util.defaultLanguage = "Java"
import demoscript.IceBox.hello

if demoscript.Util.defaultHost:
    args = ' --IceBox.UseSharedCommunicator.IceStorm=1'
else:
    args = ''

server = demoscript.Util.spawn('java IceBox.Server --Ice.Config=config.icebox --Ice.PrintAdapterReady %s' % (args))
server.expect('.* ready')
client = demoscript.Util.spawn('java Client')
client.expect('.*==>')

demoscript.IceBox.hello.run(client, server)
