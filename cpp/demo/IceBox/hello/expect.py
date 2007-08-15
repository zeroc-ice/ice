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
import demoscript.IceBox.hello

if demoscript.Util.defaultHost:
    args = ' --IceBox.UseSharedCommunicator.IceStorm=1'
else:
    args = ''

# TODO: This doesn't setup LD_LIBRARY_PATH
server = demoscript.Util.spawn('%s --Ice.Config=config.icebox --Ice.PrintAdapterReady %s' % (demoscript.Util.getIceBox(), args))
server.expect('.* ready')
client = demoscript.Util.spawn('./client')
client.expect('.*==>')

demoscript.IceBox.hello.run(client, server)
