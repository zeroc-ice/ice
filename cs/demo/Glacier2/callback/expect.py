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

import demoscript.Util
import demoscript.Glacier2.callback

server = demoscript.Util.spawn('%sserver.exe --Ice.PrintAdapterReady' % (demoscript.Util.mono()))
server.expect('.* ready')
sessionserver = demoscript.Util.spawn('%ssessionserver.exe --Ice.PrintAdapterReady' % (demoscript.Util.mono()))
sessionserver.expect('.* ready')

glacier2 = demoscript.Util.spawn('glacier2router --Ice.Config=config.glacier2 --Ice.PrintAdapterReady --Glacier2.SessionTimeout=5')
glacier2.expect('Glacier2.Client ready')
glacier2.expect('Glacier2.Server ready')

client = demoscript.Util.spawn('%sclient.exe' % (demoscript.Util.mono()))

demoscript.Glacier2.callback.run(client, server, sessionserver, glacier2)
