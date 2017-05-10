# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

props = lambda process, current: {
    "IceDiscovery.Timeout": 50,
    "IceDiscovery.RetryCount": 5,
    "IceDiscovery.Interface": "::1" if current.config.ipv6 else "127.0.0.1",
    "IceDiscovery.Port": current.driver.getTestPort(10),
    "Ice.Plugin.IceDiscovery": current.getPluginEntryPoint("IceDiscovery", process)
}

# Filter-out the warning about invalid lookup proxy
outfilters = [ lambda x: re.sub("-! .* warning: .*failed to lookup adapter.*\n", "", x),
               lambda x: re.sub("^   .*\n", "", x) ]

TestSuite(__name__, [
   ClientServerTestCase(client=Client(args=[3], props=props, outfilters=outfilters),
                        servers=[Server(args=[i], readyCount=4, props=props) for i in range(0, 3)])
], multihost=False)
