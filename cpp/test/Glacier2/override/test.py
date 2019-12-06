# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

#
# Note: we limit the send buffer size with Ice.TCP.SndSize, the
# test relies on send() blocking
#
routerProps = {
    'Ice.Warn.Dispatch' : '0',
    'Ice.Warn.Connections' : '0',
    'Ice.TCP.SndSize' : '100000',
    'Ice.ThreadPool.Server.Serialize' : '1',
    'Ice.ThreadPool.Client.Serialize' : '1',
    'Glacier2.Filter.Category.Accept' : '"c"',
    'Glacier2.PermissionsVerifier' : 'Glacier2/NullPermissionsVerifier',
    'Glacier2.Client.ForwardContext' : '1',
    'Glacier2.Client.ACM.Timeout' : '"30"',
    'Glacier2.Client.Trace.Override' : '0',
    'Glacier2.Client.Trace.Request' : '0',
    'Glacier2.Server.Trace.Override' : '0',
    'Glacier2.Server.Trace.Request' : '0',
    'Glacier2.Client.Buffered=1 --Glacier2.Server.Buffered' : '1',
    'Glacier2.Client.SleepTime=50 --Glacier2.Server.SleepTime' : '50',
}

Glacier2TestSuite(__name__, routerProps=routerProps)
