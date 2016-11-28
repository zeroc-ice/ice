# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

routerProps = lambda process, current : {
    'Ice.Warn.Dispatch' : '0',
    'Glacier2.AddConnectionContext' : '1',
    'Glacier2.Client.Endpoints' : '{0}:{1}'.format(current.getTestEndpoint(0, "tcp"), current.getTestEndpoint(1, "ssl")),
    'Ice.Admin.Endpoints' : current.getTestEndpoint(2, "tcp"),
    'Glacier2.SessionManager' : 'sessionmanager:{0}'.format(current.getTestEndpoint(3, "tcp")),
    'Glacier2.PermissionsVerifier' : 'verifier:{0}'.format(current.getTestEndpoint(3, "tcp")),
    'Glacier2.SSLSessionManager' : 'sslsessionmanager:{0}'.format(current.getTestEndpoint(3, "tcp")),
    'Glacier2.SSLPermissionsVerifier' : 'sslverifier:{0}'.format(current.getTestEndpoint(3, "tcp")),
}

#
# Always enable SSL for the Glacier2 router and client
#
sslProps = lambda process, current: current.testcase.getMapping().getSSLProps(process, current)

Glacier2TestSuite(__name__, routerProps=routerProps, options={ "ipv6" : [ False ] }, multihost=False,
                  testcases=[ClientServerTestCase(servers=[Glacier2Router(props=sslProps), Server()],
                                                  client=Client(props=sslProps))])
