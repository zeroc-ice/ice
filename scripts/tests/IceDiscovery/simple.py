# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Client properties (plugin is loaded with Ice::registerPluginFactory)
props = lambda process, current: {
    "IceDiscovery.Timeout": 50,
    "IceDiscovery.RetryCount": 5,
    "IceDiscovery.Interface": "127.0.0.1" if not current.config.ipv6 else "\"::1\"" if isinstance(platform, Darwin) else "",
    "Ice.Plugin.IceDiscovery": current.getPluginEntryPoint("IceDiscovery", process)
}

# Server properties (client properties + plugin configuration)
serverProps = lambda process, current: dict(itertools.chain({
    "Ice.Plugin.IceDiscovery": current.getPluginEntryPoint("IceDiscovery", process)
}.items(), props(process, current).items()))

TestSuite(__name__, [
   ClientServerTestCase(client=Client(args=[3], props=props),
                        servers=[Server(args=[i], readyCount=4, props=serverProps) for i in range(0, 3)])
], multihost=False)
