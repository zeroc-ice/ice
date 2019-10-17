# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

registryProps = {
    "Ice.Plugin.RegistryPlugin" : "RegistryPlugin:createRegistryPlugin",
    "IceGrid.Registry.DynamicRegistration" : 1
}
registryTraceProps = {
    "IceGrid.Registry.Trace.Locator": 2,
    "IceGrid.Registry.Trace.Server": 1,
    "IceGrid.Registry.Trace.Adapter": 1,
    "IceGrid.Registry.Trace.Node": 1,
    "Ice.Trace.Protocol": 1,
}

clientProps = {
    "Ice.RetryIntervals" : "0 50 100 250"
}
clientTraceProps = {
    "Ice.Trace.Locator": 2,
    "Ice.Trace.Protocol": 1
}

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__,
              [IceGridTestCase(icegridregistry=[IceGridRegistryMaster(props=registryProps, traceProps=registryTraceProps)],
                               client=IceGridClient(props=clientProps, traceProps=clientTraceProps))],
              libDirs=["registryplugin", "testservice"],
              multihost=False)
