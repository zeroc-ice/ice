# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from IceBridgeUtil import IceBridge
from Util import ClientServerTestCase, Server, TestSuite


TestSuite(__file__, [ClientServerTestCase(servers=[IceBridge(), Server()])])
