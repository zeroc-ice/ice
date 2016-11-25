# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

TestSuite(__name__, [
    IceGridTestCase("without deployment", application=None,
                    icegridregistry=[IceGridRegistryMaster(props=registryProps),
                                     IceGridRegistrySlave(1, props=registryProps),
                                     IceGridRegistrySlave(2, props=registryProps)],
                    client=ClientServerTestCase(client=IceGridClient(), server=IceGridServer(props=serverProps))),
    IceGridTestCase("with deployment", client=IceGridClient(args=["--with-deploy"]))
], multihost=False)
