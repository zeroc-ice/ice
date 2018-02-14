# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

serverProps = {
    "TestAdapter.Endpoints" : "default",
    "TestAdapter.AdapterId" : "TestAdapter"
}

registryProps = {
    "IceGrid.Registry.DynamicRegistration" : 1
}
registryTraceProps = {
    "IceGrid.Registry.Trace.Discovery": 2
}

clientProps = lambda process, current: {
    "IceLocatorDiscovery.Timeout": 50,
    "IceLocatorDiscovery.RetryCount": 5,
    "IceLocatorDiscovery.Interface": "" if isinstance(platform, Linux) else "::1" if current.config.ipv6 else "127.0.0.1",
    "IceLocatorDiscovery.Port": current.driver.getTestPort(99),
}
clientTraceProps = { "IceLocatorDiscovery.Trace.Lookup" : 3 }

# Filter-out the warning about invalid lookup proxy
outfilters = [ lambda x: re.sub("-! .* warning: .*failed to lookup locator.*\n", "", x),
               lambda x: re.sub("^   .*\n", "", x) ]

TestSuite(__name__, [
    IceGridTestCase("without deployment", application=None,
                    icegridregistry=[IceGridRegistryMaster(props=registryProps, traceProps=registryTraceProps),
                                     IceGridRegistrySlave(1, props=registryProps, traceProps=registryTraceProps),
                                     IceGridRegistrySlave(2, props=registryProps, traceProps=registryTraceProps)],
                    client=ClientServerTestCase(client=IceGridClient(props=clientProps,
                                                                     outfilters=outfilters,
                                                                     traceProps=clientTraceProps),
                                                server=IceGridServer(props=serverProps))),
    IceGridTestCase("with deployment", client=IceGridClient(args=["--with-deploy"]))
], multihost=False)
