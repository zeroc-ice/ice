# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
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
