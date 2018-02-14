# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Note: we limit the send buffer size with Ice.TCP.SndSize, the
# test relies on send() blocking
#
routerProps = lambda process, current : {
    'Glacier2.SessionManager' : 'SessionManager:{0}'.format(current.getTestEndpoint(0)),
    'Glacier2.PermissionsVerifier' : 'Glacier2/NullPermissionsVerifier',
    'Ice.Default.Locator' : 'locator:{0}'.format(current.getTestEndpoint(1)),
}

Glacier2TestSuite(__name__,
                  testcases=[ClientServerTestCase(servers=[Glacier2Router(props=routerProps, passwords=None),
                                                  Server(readyCount=3)])])
