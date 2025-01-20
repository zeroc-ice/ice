# Copyright (c) ZeroC, Inc.

from Util import ClientTestCase, SimpleClient, TestSuite


TestSuite(__name__, [ClientTestCase(client=SimpleClient(args=["{testdir}"]))])
