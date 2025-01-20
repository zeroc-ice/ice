# Copyright (c) ZeroC, Inc.

from IceBridgeUtil import IceBridge
from Util import ClientServerTestCase, Server, TestSuite


TestSuite(__file__, [ClientServerTestCase(servers=[IceBridge(), Server()])])
