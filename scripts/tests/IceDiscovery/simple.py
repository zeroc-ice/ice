# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Client properties (plugin is loaded with Ice::registerPluginFactory)
#
# NOTE: we use the loopback interface for multicast with IPv6 to prevent failures
# on some machines which don't really have an IPv6 interface configured.
#
props = lambda process, current: {
    "IceDiscovery.Timeout": 50,
    "IceDiscovery.RetryCount": 5,
    "Ice.Plugin.IceDiscovery": current.getPluginEntryPoint("IceDiscovery", process),
    "Ice.Trace.Network": 2,
    "Ice.Trace.Locator": 2,
}

# Server properties (client properties + plugin configuration)
serverProps = lambda process, current: dict(itertools.chain({
    "Ice.Plugin.IceDiscovery": current.getPluginEntryPoint("IceDiscovery", process)
}.items(), props(process, current).items()))

TestSuite(__name__, [
   ClientServerTestCase(client=Client(args=[3], props=props),
                        servers=[Server(args=[i], readyCount=4, props=serverProps) for i in range(0, 3)])
], multihost=False)
