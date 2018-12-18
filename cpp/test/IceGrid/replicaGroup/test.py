# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

registryProps = {
    "Ice.Plugin.RegistryPlugin" : "RegistryPlugin:createRegistryPlugin",
    "IceGrid.Registry.DynamicRegistration" : 1
}

clientProps = {
    "Ice.RetryIntervals" : "0 50 100 250"
}

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__,
              [IceGridTestCase(icegridregistry=[IceGridRegistryMaster(props=registryProps)],
                               client=IceGridClient(props=clientProps))],
              libDirs=["registryplugin", "testservice"],
              multihost=False)
